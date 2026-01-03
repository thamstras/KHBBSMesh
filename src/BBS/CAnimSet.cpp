#include "CAnimSet.h"
#include <set>
#include <functional>
#include <algorithm>
#include <format>


using namespace BBS;

float InterpVal(std::vector<Keyframe<float>> kfs, float t)
{
	if (kfs.size() == 1) return kfs[0].value;

	int i = kfs.size() - 1;
	for (; i > 0; i--)
		if (kfs[i].time <= t) break;

	if (i == kfs.size() - 1) return kfs[i].value;
	
	float t1 = kfs[i].time;
	float t2 = kfs[i + 1].time;
	return std::lerp(kfs[i].value, kfs[i + 1].value, (t - t1) / (t2 - t1));
}

template <typename T>
std::vector<Keyframe<T>> BuildChannel(std::vector<Keyframe<float>> x, std::vector<Keyframe<float>> y, std::vector<Keyframe<float>> z, std::function<T(float, float, float)> factory)
{
	std::vector<Keyframe<T>> keyList = std::vector<Keyframe<T>>();

	// make an ordered set of all frameNumbers, then iterate through it.
	std::set<float> targetFrames = std::set<float>();
	for (auto& kf : x) targetFrames.insert(kf.time);
	for (auto& kf : y) targetFrames.insert(kf.time);
	for (auto& kf : z) targetFrames.insert(kf.time);

	for (float currFrame : targetFrames)
	{
		keyList.emplace_back(currFrame, factory(InterpVal(x, currFrame), InterpVal(y, currFrame), InterpVal(z, currFrame)));
	}

	return keyList;
}


VectorChannel::VectorChannel(std::vector<Keyframe<float>> x, std::vector<Keyframe<float>> y, std::vector<Keyframe<float>> z)
{
	keyframes = BuildChannel<glm::vec3>(x, y, z, [](float a, float b, float c) { return glm::vec3(a, b, c); });
}

glm::vec3 VectorChannel::Evaluate(float time)
{
	int previ = FindPrevKeyframeIdx(time);
	Keyframe<glm::vec3> kf1 = keyframes[previ];
	if (previ == keyframes.size() - 1) return kf1.value;
	Keyframe<glm::vec3> kf2 = keyframes[previ + 1];
	float w = kf2.time - kf1.time;
	return glm::mix(kf1.value, kf2.value, (time - kf1.time) / w);
}

QuatChannel::QuatChannel(std::vector<Keyframe<float>> x, std::vector<Keyframe<float>> y, std::vector<Keyframe<float>> z)
{
	keyframes = BuildChannel<glm::quat>(x, y, z, [](float a, float b, float c) { return glm::quat(glm::vec3(a, b, c)); });
}

glm::quat QuatChannel::Evaluate(float time)
{
	int previ = FindPrevKeyframeIdx(time);
	Keyframe<glm::quat> kf1 = keyframes[previ];
	if (previ == keyframes.size() - 1) return kf1.value;
	Keyframe<glm::quat> kf2 = keyframes[previ + 1];
	float w = kf2.time - kf1.time;
	return glm::slerp(kf1.value, kf2.value, (time - kf1.time) / w);
}

Keyframe<float> TransformKF(PamKeyframe kf, float min, float max)
{
	float dv;
	if (kf.value == 0) dv = min;
	else if (kf.value == UINT16_MAX) dv = max;
	else
	{
		float range = max - min;
		float factor = (float)kf.value / (float)UINT16_MAX;
		dv = min + (range * factor);
	}

	return Keyframe<float>(kf.keyframeId, dv);
}

// TODO: Make this a free function so BoneChannel can be generic
BoneChannel::BoneChannel(PamBoneAnim& pamBone)
{
	auto f = [](std::optional<PamAnimChannel> c, float d)
		{
			std::vector<Keyframe<float>> kfv{};
			if (!c.has_value())
			{
				kfv.emplace_back(0.0f, d);
			}
			else
			{
				//auto kff = std::bind(TransformKF, std::placeholders::_1, c->minVal, c->maxVal);
				//std::ranges::transform(c->keyframes, std::back_inserter(kfv), kff);
				
				if (c->keyframeCount == 1)
				{
					kfv.push_back(Keyframe<float>(0.0f, c->minVal));
				}
				else
				{
					for (auto& k : c->keyframes)
						kfv.push_back(TransformKF(k, c->minVal, c->maxVal));
				}
			}
			return kfv;
		};

	translate = std::make_unique<VectorChannel>(f(pamBone.translate_x, 0.0f), f(pamBone.translate_y, 0.0f), f(pamBone.translate_z, 0.0f));
	rotate = std::make_unique<QuatChannel>(f(pamBone.rotate_x, 0.0f), f(pamBone.rotate_y, 0.0f), f(pamBone.rotate_z, 0.0f));
	scale = std::make_unique<VectorChannel>(f(pamBone.scale_x, 1.0f), f(pamBone.scale_y, 1.0f), f(pamBone.scale_z, 1.0f));
}

BoneFrame BoneChannel::Evaluate(float time) const
{
	glm::vec3 t = translate->Evaluate(time);
	glm::quat r = rotate->Evaluate(time);
	glm::vec3 s = scale->Evaluate(time);
	glm::mat4 m = glm::translate(glm::mat4(1.0f), t);
	m = glm::rotate(m, glm::angle(r), glm::axis(r));
	glm::mat4 m2 = glm::scale(m, s);
	return { m2, m, s };
}

Transform BoneChannel::GetTransform(float time) const
{
	glm::vec3 t = translate->Evaluate(time);
	glm::quat r = rotate->Evaluate(time);
	glm::vec3 s = scale->Evaluate(time);
	return { t, glm::eulerAngles(r), s};
}

VectorChannel const* BoneChannel::GetRawTranslate() const
{
	return translate.get();
}

QuatChannel const* BoneChannel::GetRawRotate() const
{
	return rotate.get();
}

VectorChannel const* BoneChannel::GetRawScale() const
{
	return scale.get();
}

Anim::Anim(PamAnim& source)
{
	frameRate = source.frameRate;
	frameCount = source.frameCount;
	loopFrom = source.loopFromFrame;
	loopTo = source.loopToFrame;
	for (auto& b : source.boneAnims)
		bones.emplace_back(b);
}

int Anim::BoneCount() const { return bones.size(); }

int Anim::FrameRate() const { return frameRate; }

int Anim::FrameCount() const { return frameCount; }

int Anim::LoopFrom() const { return loopFrom; }

int Anim::LoopTo() const { return loopTo; }

void Anim::ModFrameRate(int newFrameRate)
{
	frameRate = newFrameRate;
}

void Anim::ModLoopFrom(int newLoopFrom)
{
	int frameCount = FrameCount();
	newLoopFrom = std::clamp(newLoopFrom, 0, frameCount);
	loopFrom = newLoopFrom;
}

void Anim::ModLoopTo(int newLoopTo)
{
	int frameCount = FrameCount();
	newLoopTo = std::clamp(newLoopTo, 0, frameCount);
	loopFrom = newLoopTo;
}

BoneFrame Anim::GetBone(float fTime, int boneIdx)
{
	if (boneIdx < 0 || boneIdx >= BoneCount())
		return {};	// TODO: Check if this is 0 or identity

	return bones[boneIdx].Evaluate(fTime);
}

BoneChannel const* Anim::GetRaw(int boneIdx) const
{
	return &bones[boneIdx];
}

CBBSAnimationProvider::CBBSAnimationProvider(PamFile& pamFile, CSkeleton skeleton) : skeleton(skeleton)
{
	for (auto& anim : pamFile.contents)
	{
		AnimInfo info;
		info.name = std::string(anim.name, 12);
		if (anim.data.has_value())
		{
			info.storeIdx = anims.size();
			anims.emplace_back(anim.data.value());
		}
		animInfos.push_back(info);
	}
	SelectAnim(0);
	SetPlayRate(1.0f);
	SetPlaying(false);
}

CBBSAnimationProvider::~CBBSAnimationProvider() {}

int CBBSAnimationProvider::BoneCount()
{
	AnimInfo& currAnim = animInfos[selectedIdx];
	if (currAnim.storeIdx != -1)
		return anims[currAnim.storeIdx].BoneCount();
	return 0;
}

void CBBSAnimationProvider::Update(float deltaTime, double worldTime)
{
	if (isPlaying && selectedAnim != nullptr)
	{
		currTime += deltaTime * playbackRate;
		CalcFrame();
	}
}

BoneFrame CBBSAnimationProvider::GetBone(int idx)
{
	BoneFrame boneTransform = { skeleton.bones[idx].transform, skeleton.bones[idx].transform };
	if (selectedAnim != nullptr)
		boneTransform = selectedAnim->GetBone(currFrame, idx);

	return boneTransform;
}

void CBBSAnimationProvider::SetAnimTime(float time)
{
	currTime = std::max(time, 0.0f);
	CalcFrame();
}

void CBBSAnimationProvider::SetPlayRate(float rate)
{
	playbackRate = std::clamp(rate, 0.0001f, 1000.0f);
}

void CBBSAnimationProvider::SetPlaying(bool isPlaying)
{
	this->isPlaying = isPlaying;
}

bool CBBSAnimationProvider::NeedsScaleHack() { return true; }

int CBBSAnimationProvider::AnimCount() const { return animInfos.size(); }

void CBBSAnimationProvider::SelectAnim(int idx)
{
	selectedIdx = std::clamp(idx, 0, AnimCount());
	if (animInfos[selectedIdx].storeIdx >= 0)
		selectedAnim = &anims[animInfos[selectedIdx].storeIdx];
	else
		selectedAnim = nullptr;
	SetAnimTime(0.0f);
}

std::string CBBSAnimationProvider::GetCurrAnimName() const { return animInfos[selectedIdx].name; }

Anim const* CBBSAnimationProvider::GetCurrAnim() const { return selectedAnim; };

void CBBSAnimationProvider::SetAnimFrame(float frame)
{
	if (selectedAnim == nullptr) return;

	int frameRate = selectedAnim->FrameRate();
	float time = frame / (float)frameRate;
	SetAnimTime(time);
}

void CBBSAnimationProvider::CalcFrame()
{
	if (selectedAnim == nullptr) return;

	if (currTime < 0.0f) currTime = 0.0f;
	currFrame = currTime * (float)selectedAnim->FrameRate();

	if (shouldLoop)
	{
		int lf = selectedAnim->LoopFrom();
		// if no loopFrom loop whole anim
		if (lf == 0) lf = selectedAnim->FrameCount();
		if (currFrame >= (float)lf)
		{
			float loopLen = (float)(lf - selectedAnim->LoopTo());
			currFrame -= loopLen;
			currTime -= loopLen / selectedAnim->FrameRate();
		}
	}

	if (currFrame > (float)selectedAnim->FrameCount())
	{
		currFrame = selectedAnim->FrameCount();
		currTime = (float)selectedAnim->FrameCount() / selectedAnim->FrameRate();
	}
}

void CBBSAnimationProvider::GUI_Controls()
{
	ImGui::Text("Playback Controls");
	float t = currTime;
	if (ImGui::InputFloat("Play Time", &t))
		SetAnimTime(t);
	t = this->playbackRate;
	if (ImGui::InputFloat("Play Rate", &t))
		SetPlayRate(t);
	bool play = this->isPlaying;
	if (ImGui::Checkbox("Play", &play))
		SetPlaying(play);
	ImGui::Separator();


	if (ImGui::BeginCombo("Curr Anim", animInfos[selectedIdx].name.c_str()))
	{
		for (int i = 0; i < animInfos.size(); i++)
		{
			auto label = std::format("{} : {}", i, animInfos[i].storeIdx >= 0 ? animInfos[i].name.c_str() : "NO ANIM");
			if (ImGui::Selectable(label.c_str(), selectedIdx == i))
				SelectAnim(i);
		}
		ImGui::EndCombo();
	}

	if (selectedAnim == nullptr)
	{
		ImGui::Text("No Anim in this slot!");
	}
	else
	{
		ImGui::Text("Anim Time %f", currTime);
		ImGui::Text("Fame Rate %d", selectedAnim->FrameRate());
		ImGui::Text("Curr Frame %d", (int)currFrame);
		ImGui::Text("Frame Count %d", selectedAnim->FrameCount());
		ImGui::Text("Loop From %d", selectedAnim->LoopFrom());
		ImGui::Text("Loop To %d", selectedAnim->LoopTo());
		ImGui::Text("Bone Count %d", selectedAnim->BoneCount());
		ImGui::Checkbox("Loop?", &shouldLoop);
		ImGui::Separator();
		if (ImGui::Button("Prev Frame"))
		{
			SetAnimFrame(currFrame - 1);
		}
		ImGui::SameLine();
		if (ImGui::Button("Next Frame"))
		{
			SetAnimFrame(currFrame + 1);
		}
		ImGui::Checkbox("Show Pose?", &gui_showPose);
		if (gui_showPose)
		{
			ImGui::Separator();
			int bcount = BoneCount();
			for (int i = 0; i < bcount; i++)
			{
				CBone& bone = skeleton.bones[i];
				Transform t = selectedAnim->GetRaw(i)->GetTransform(currFrame);
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