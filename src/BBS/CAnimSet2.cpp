#include "CAnimSet2.h"
#include <set>
#include <functional>
#include <algorithm>


using namespace BBS;

float InterpVal(std::vector<Keyframe<float>> kfs, float t)
{
	int i = kfs.size();
	for (; i > 0; i--)
		if (kfs[i].time <= t) break;
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
				auto kff = std::bind(TransformKF, std::placeholders::_1, c->minVal, c->maxVal);
				std::ranges::transform(c->keyframes, std::back_inserter(kfv), kff);
			}
			return kfv;
		};

	translate = std::make_unique<VectorChannel>(f(pamBone.translate_x, 0.0f), f(pamBone.translate_y, 0.0f), f(pamBone.translate_z, 0.0f));
	rotate = std::make_unique<QuatChannel>(f(pamBone.rotate_x, 0.0f), f(pamBone.rotate_y, 0.0f), f(pamBone.rotate_z, 0.0f));
	scale = std::make_unique<VectorChannel>(f(pamBone.scale_x, 1.0f), f(pamBone.scale_y, 1.0f), f(pamBone.scale_z, 1.0f));
}

