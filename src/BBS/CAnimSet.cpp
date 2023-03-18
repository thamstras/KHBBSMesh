#include "CAnimSet.h"
#include <cmath>
#include <format>
#include "Core/Transform.h"

using namespace BBS;

IAnimChannel::IAnimChannel()
{

}

CConstChannel::CConstChannel(float value)
{
	theValue = value;
}

float CConstChannel::Evaluate(int frame)
{
	return theValue;
}

CKeyframeChannel::CKeyframeChannel(std::vector<Keyframe> frames)
{
	this->keyframes = frames;
}

float CKeyframeChannel::Evaluate(int frame)
{
	int keyIdx = FindPrevKey(frame);

	// TODO: Bounds Check

	Keyframe& prev = keyframes[keyIdx];
	Keyframe& next = keyframes[keyIdx + 1];

	if (prev.value == next.value) return prev.value;

	int width = next.frameNumber - prev.frameNumber;
	float frac = (frame - prev.frameNumber) / (float)width;

	return std::lerp(prev.value, next.value, frac);
}

int CKeyframeChannel::FindPrevKey(int frame)
{
	for (int i = keyframes.size() - 2; i > 0; i--)
	{
		if (keyframes[i].frameNumber <= frame)
		{
			return i;
		}
	}
	return 0;
}

CBoneAnim::CBoneAnim(PamBoneAnim& pamBone)
{
	tx = MakeChannel(pamBone.translate_x);
	ty = MakeChannel(pamBone.translate_y);
	tz = MakeChannel(pamBone.translate_z);
	rx = MakeChannel(pamBone.rotate_x);
	ry = MakeChannel(pamBone.rotate_y);
	rz = MakeChannel(pamBone.rotate_z);
	sx = MakeChannel(pamBone.scale_x, 1.0f);
	sy = MakeChannel(pamBone.scale_y, 1.0f);
	sz = MakeChannel(pamBone.scale_z, 1.0f);
}

float DequantizeShort(uint16_t value, float min, float max)
{
	if (max < min) throw new std::invalid_argument("max must not be less than min");
	if (isnan(max)) throw new std::domain_error("max must not be nan");
	if (isnan(min)) throw new std::domain_error("min must not be nan");

	if (value == 0) return min;
	if (value == UINT16_MAX) return max;

	float range = max - min;
	float factor = (float)value / (float)UINT16_MAX;
	return min + (range * factor);
}

std::unique_ptr<IAnimChannel> CBoneAnim::MakeChannel(std::optional<PamAnimChannel>& pamChannel, float defaultVal)
{
	if (!pamChannel.has_value()) return std::make_unique<CConstChannel>(defaultVal);

	if (pamChannel->keyframeCount == 1) return std::make_unique<CConstChannel>(pamChannel->minVal);

	std::vector<Keyframe> keyframes = std::vector<Keyframe>();
	for (auto& pamKey : pamChannel->keyframes)
	{
		Keyframe frame{};
		frame.frameNumber = pamKey.keyframeId;
		frame.value = DequantizeShort(pamKey.value, pamChannel->minVal, pamChannel->maxVal);
		keyframes.push_back(frame);
	}
	return std::make_unique<CKeyframeChannel>(keyframes);
}

glm::mat4 CBoneAnim::Evaluate(int frame)
{
	glm::vec3 translate = glm::vec3(tx->Evaluate(frame), ty->Evaluate(frame), tz->Evaluate(frame));
	glm::vec3 rotate = glm::vec3(rx->Evaluate(frame), ry->Evaluate(frame), rz->Evaluate(frame));
	glm::vec3 scale = glm::vec3(sx->Evaluate(frame), sy->Evaluate(frame), sz->Evaluate(frame));

	glm::quat rquat = glm::quat(rotate);

	glm::mat4 t = glm::mat4(1.0f);
	t = glm::translate(t, translate);
	t = glm::rotate(t, glm::angle(rquat), glm::axis(rquat));
	t = glm::scale(t, scale);

	return t;
}

CBBSAnim::CBBSAnim(PamAnim& pamAnim)
{
	frameRate = pamAnim.frameRate;
	frameCount = pamAnim.frameCount;
	loopFrom = pamAnim.loopFromFrame;
	loopTo = pamAnim.loopToFrame;
	boneCount = pamAnim.boneCount;
	for (auto& bone : pamAnim.boneAnims)
		bones.emplace_back(bone);	// Calls CBoneAnim in place!
}

void CBBSAnim::CalcFrame()
{
	if (currTime < 0.0f) currTime = 0.0f;

	float fFrame = currTime * frameRate;

	if (shouldLoop)
	{
		if (loopFrom > 0)
		{
			if (fFrame > (float)loopFrom)
			{
				float loopLen = (float)loopFrom - loopTo;
				fFrame -= loopLen;
				currTime -= loopLen / frameRate;
			}
		}
		else
		{
			// This anim probably isn't supposed to loop so we'll loop all of it
			if (fFrame > (float)frameCount)
			{
				fFrame = 0.0f;
				currTime = 0.0f;
			}
		}
	}

	if (fFrame > (float)frameCount)
	{
		fFrame = frameCount;
		currTime = (float)frameCount / frameRate;
	}

	currFrame = (int)floor(fFrame);
}

void CBBSAnim::Update(float animDeltaTime)
{
	currTime += animDeltaTime;

	CalcFrame();
}

void CBBSAnim::SetTime(float time)
{
	currTime = time;
	CalcFrame();
}

glm::mat4 CBBSAnim::GetBone(int boneIdx)
{
	if (boneIdx >= boneCount) return glm::mat4(1.0f);
	return bones[boneIdx].Evaluate(currFrame);
}

CBBSAnimSet::CBBSAnimSet(PamFile& pamFile, CSkeleton skeleton) : skeleton(skeleton)
{
	for (auto& anim : pamFile.contents)
	{
		AnimInfo info;
		info.name = std::string(anim.name, 12);
		if (anim.data.has_value())
		{
			info.idx = anims.size();
			anims.emplace_back(anim.data.value());
		}
		else
		{
			info.idx = -1;
		}
		animInfos.push_back(info);
	}
}

CBBSAnimSet::~CBBSAnimSet() {}

int CBBSAnimSet::BoneCount()
{
	AnimInfo& currAnim = animInfos[selectedIdx];
	if (currAnim.idx != -1)
		return anims[currAnim.idx].boneCount;
	return 0;
}

void CBBSAnimSet::Update(float deltaTime, double worldTime)
{
	if (isPlaying)
	{
		currAnimTime += deltaTime * timescale;
		AnimInfo& currAnim = animInfos[selectedIdx];
		if (currAnim.idx != -1)
			anims[currAnim.idx].Update(deltaTime * timescale);
	}
}

glm::mat4 CBBSAnimSet::GetBone(int idx)
{
	AnimInfo& currAnim = animInfos[selectedIdx];
	glm::mat4 boneTransform = skeleton.bones[idx].transform;
	if (currAnim.idx != -1)
		boneTransform = anims[currAnim.idx].GetBone(idx);

	return boneTransform;
}

void CBBSAnimSet::SetAnimTime(float time)
{
	currAnimTime = time;
	AnimInfo& currAnim = animInfos[selectedIdx];
	if (currAnim.idx != -1)
		anims[currAnim.idx].SetTime(currAnimTime);
}

void CBBSAnimSet::SetPlayRate(float rate)
{
	timescale = rate;
}

void CBBSAnimSet::SetPlaying(bool isPlaying)
{
	this->isPlaying = isPlaying;
}

bool CBBSAnimSet::NeedsScaleHack()
{
	return true;
}

int CBBSAnimSet::AnimCount()
{
	return animInfos.size();
}

void CBBSAnimSet::SelectAnim(int idx)
{
	selectedIdx = idx;
}

void CBBSAnimSet::GUI_DrawControls()
{
	ImGui::Text("Playback Controls");
	//ImGui::SetNextItemWidth(0.4f);
	float t = this->currAnimTime;
	if (ImGui::InputFloat("Play Time", &t))
		SetAnimTime(t);
	//ImGui::SameLine();
	//ImGui::SetNextItemWidth(0.4f);
	t = this->timescale;
	if (ImGui::InputFloat("Play Rate", &t))
		SetPlayRate(t);
	bool play = this->isPlaying;
	if (ImGui::Checkbox("Play", &play))
		SetPlaying(play);
	ImGui::Separator();
	
	
	if(ImGui::BeginCombo("Curr Anim", animInfos[selectedIdx].name.c_str()))
	{
		for (int i = 0; i < animInfos.size(); i++)
		{
			auto label = std::format("{} : {}", i, animInfos[i].idx != -1 ? animInfos[i].name.c_str() : "NO ANIM");
			if (ImGui::Selectable(label.c_str(), selectedIdx == i))
				SelectAnim(i);
		}
		ImGui::EndCombo();
	}

	if (animInfos[selectedIdx].idx == -1)
	{
		ImGui::Text("No Anim in this slot!");
	}
	else
	{
		CBBSAnim& anim = anims[animInfos[selectedIdx].idx];
		ImGui::Text("Anim Time %f", anim.currTime);
		ImGui::Text("Fame Rate %d", anim.frameRate);
		ImGui::Text("Curr Frame %d", anim.currFrame);
		ImGui::Text("Frame Count %d", anim.frameCount);
		ImGui::Text("Loop From %d", anim.loopFrom);
		ImGui::Text("Loop To %d", anim.loopTo);
		ImGui::Text("Bone Count %d", anim.boneCount);
		ImGui::Checkbox("Loop?", &anim.shouldLoop);
		ImGui::Separator();
		if (ImGui::Button("Prev Frame"))
		{
			SetAnimTime((anim.currFrame - 1) / (float)anim.frameRate);
		}
		ImGui::SameLine();
		if (ImGui::Button("Next Frame"))
		{
			SetAnimTime((anim.currFrame + 1) / (float)anim.frameRate);
		}
		ImGui::Checkbox("Show Pose?", &gui_showPose);
		if (gui_showPose)
		{
			ImGui::Separator();
			int bcount = BoneCount();
			for (int i = 0; i < bcount; i++)
			{
				CBone& bone = skeleton.bones[i];
				Transform t = Transform::Decompose(GetBone(i));
				ImGui::PushID(i);
				ImGui::Text("Bone: %s", bone.name.c_str());
				ImGui::DragFloat3("Position", glm::value_ptr(t.position), 0.1f);
				ImGui::DragFloat3("Rotation", glm::value_ptr(t.rotation), 0.1f, -180.0f, 180.0f);
				ImGui::DragFloat3("Scale", glm::value_ptr(t.scale), 0.1f);
				ImGui::Separator();
				ImGui::PopID();
			}
		}
	}
}