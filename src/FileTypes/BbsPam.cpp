#include "BbsPam.h"
#include <stdexcept>
#include "..\Utils\StreamUtils.h"
#include "..\Utils\BitCast.h"
#include "..\Utils\MagicCode.h"

constexpr uint32_t pamMagic = MagicCode('P', 'A', 'M', '\0');

PamChannelFlags ParsePamChannelFlags(std::ifstream& file)
{
	uint16_t raw;
	PamChannelFlags flags;
	raw = ReadStream<uint16_t>(file);
	flags = bit_cast<PamChannelFlags, uint16_t>(raw);
	return flags;
}

PamKeyframe ParsePamKeyframe(std::ifstream& file, uint16_t frameCount)
{
	PamKeyframe keyframe{};
	if (frameCount <= 255)
		keyframe.keyframeId = (uint16_t)ReadStream<uint8_t>(file);
	else		
		keyframe.keyframeId = ReadStream<uint16_t>(file);
	ReadStream(file, keyframe.value);
	return keyframe;
}

PamAnimChannel ParsePamAnimChannel(std::ifstream& file, uint16_t frameCount)
{
	PamAnimChannel channel{};
	channel.maxVal = ReadStream<float>(file);
	channel.minVal = ReadStream<float>(file);
	if (frameCount <= 255)
		channel.keyframeCount = (uint16_t)ReadStream<uint8_t>(file);
	else
		channel.keyframeCount = ReadStream<uint16_t>(file);
	// Degenerate condition; If frameCount == 1 && keyframeCount == 1 we would read a value here we shouldn't.
	if (channel.keyframeCount == frameCount && frameCount > 1)
	{
		for (int k = 0; k < frameCount; k++)
		{
			PamKeyframe key{};
			key.keyframeId = k;
			key.value = ReadStream<uint16_t>(file);
			channel.keyframes.push_back(key);
		}
	}
	else if (channel.keyframeCount > 1)
	{
		for (int k = 0; k < channel.keyframeCount; k++)
			channel.keyframes.push_back(ParsePamKeyframe(file, frameCount));
	}
	// else keyframeCount == 1 => constant value, no keyframe data
	return channel;
}

PamBoneAnim ParsePamBoneAnim(std::ifstream& file, uint16_t frameCount, PamChannelFlags& flags)
{
	PamBoneAnim bone{};
	if (flags.translate_x) bone.translate_x = ParsePamAnimChannel(file, frameCount);
	if (flags.translate_y) bone.translate_y = ParsePamAnimChannel(file, frameCount);
	if (flags.translate_z) bone.translate_z = ParsePamAnimChannel(file, frameCount);
	if (flags.rotate_x) bone.rotate_x = ParsePamAnimChannel(file, frameCount);
	if (flags.rotate_y) bone.rotate_y = ParsePamAnimChannel(file, frameCount);
	if (flags.rotate_z) bone.rotate_z = ParsePamAnimChannel(file, frameCount);
	if (flags.scale_x) bone.scale_x = ParsePamAnimChannel(file, frameCount);
	if (flags.scale_y) bone.scale_y = ParsePamAnimChannel(file, frameCount);
	if (flags.scale_z) bone.scale_z = ParsePamAnimChannel(file, frameCount);
	return bone;
}

PamAnim ParsePamAnim(std::ifstream& file)
{
	PamAnim anim{};
	anim.animType = ReadStream<uint16_t>(file);
	anim.frameRate = ReadStream<uint8_t>(file);
	anim.interpFrameCount = ReadStream<uint8_t>(file);
	anim.loopFromFrame = ReadStream<uint16_t>(file);
	anim.boneCount = ReadStream<uint8_t>(file);
	anim.padding = ReadStream<int8_t>(file);
	anim.frameCount = ReadStream<uint16_t>(file);
	anim.loopToFrame = ReadStream<uint16_t>(file);
	for (int b = 0; b < anim.boneCount; b++)
		anim.channelFlags.push_back(ParsePamChannelFlags(file));
	for (int b = 0; b < anim.boneCount; b++)
		anim.boneAnims.push_back(ParsePamBoneAnim(file, anim.frameCount, anim.channelFlags[b]));
	return anim;
}

PamAnimEntry ParsePamAnimEntry(std::ifstream& file)
{
	PamAnimEntry entry{};
	entry.offset = ReadStream<uint32_t>(file);
	ReadStreamArr<char>(file, entry.name, 12);
	return entry;
}

PamHeader ParsePamHeader(std::ifstream& file)
{
	PamHeader header{};
	header.magic = ReadStream<uint32_t>(file);
	if (header.magic != pamMagic)
	{
		throw std::runtime_error("File is not a valid PAM file! (Magic code fail)");
	}
	header.animationCount = ReadStream<uint32_t>(file);
	ReadStreamArr(file, header.padding, 6);
	header.version = ReadStream<uint16_t>(file);
	return header;
}

PamFile PamFile::ReadPamFile(std::ifstream& file, std::streamoff base)
{
	PamFile pam;
	pam.header = ParsePamHeader(file);
	for (int e = 0; e < (int)pam.header.animationCount; e++)
		pam.contents.push_back(ParsePamAnimEntry(file));
	for (auto& entry : pam.contents)
	{
		if (entry.offset != 0)
		{
			file.seekg(base + (std::streamoff)entry.offset, std::ios_base::beg);
			entry.data = ParsePamAnim(file);
		}
	}
	return pam;
}