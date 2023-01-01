#pragma once
#include <vector>
#include <fstream>
#include <cstdint>
#include <optional>

struct PamChannelFlags
{
	uint16_t translate_x : 1;
	uint16_t translate_y : 1;
	uint16_t translate_z : 1;
	uint16_t rotate_x : 1;
	uint16_t rotate_y : 1;
	uint16_t rotate_z : 1;
	uint16_t scale_x : 1;
	uint16_t scale_y : 1;
	uint16_t scale_z : 1;
};

struct PamKeyframe
{
	uint16_t keyframeId;
	uint16_t value;
};

struct PamAnimChannel
{
	float maxVal;
	float minVal;
	uint16_t keyframeCount;

	std::vector<PamKeyframe> keyframes;
};

struct PamBoneAnim
{
	std::optional<PamAnimChannel> translate_x;
	std::optional<PamAnimChannel> translate_y;
	std::optional<PamAnimChannel> translate_z;
	std::optional<PamAnimChannel> rotate_x;
	std::optional<PamAnimChannel> rotate_y;
	std::optional<PamAnimChannel> rotate_z;
	std::optional<PamAnimChannel> scale_x;
	std::optional<PamAnimChannel> scale_y;
	std::optional<PamAnimChannel> scale_z;
};

struct PamAnim
{
	uint16_t animType;
	uint8_t frameRate;
	uint8_t interpFrameCount;
	uint16_t loopFromFrame;
	uint8_t boneCount;
	int8_t padding;
	uint16_t frameCount;
	uint16_t loopToFrame;

	std::vector<PamChannelFlags> channelFlags;
	std::vector<PamBoneAnim> boneAnims;
};

struct PamAnimEntry
{
	uint32_t offset;
	char name[12];

	std::optional<PamAnim> data;
};

struct PamHeader
{
	uint32_t magic;
	uint32_t animationCount;
	int8_t padding[6];
	uint16_t version;
};

class PamFile
{
public:
	PamHeader header;
	std::vector<PamAnimEntry> contents;

	static PamFile ReadPamFile(std::ifstream& file, std::streamoff base = 0);
};