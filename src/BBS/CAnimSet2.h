#pragma once
#include "Common.h"
#include "Core/CAnimationDriver.h"
#include "FileTypes/BbsPam.h"

class AssimpAnimExporter;

namespace BBS
{
	template <typename T>
	struct Keyframe
	{
		float time;
		T value;

		Keyframe(float t, T v) : time(t), value(v) {}
	};

	template <typename T>
	class KeyChannel
	{
	public:
		KeyChannel() = default;
		virtual ~KeyChannel() = default;

		virtual T Evaluate(float time) = 0;

	protected:
		std::vector<Keyframe<T>> keyframes;
		int FindPrevKeyframeIdx(float t)
		{
			for (int i = keyframes.size() - 2; i > 0; i--)
			{
				if (keyframes[i].time <= t)
				{
					return i;
				}
			}
			return 0;
		}
	};

	class VectorChannel : public KeyChannel<glm::vec3>
	{
	public:
		VectorChannel(std::vector<Keyframe<float>> x, std::vector<Keyframe<float>> y, std::vector<Keyframe<float>> z);
		virtual ~VectorChannel() = default;

		glm::vec3 Evaluate(float time) override;
	};

	class QuatChannel : public KeyChannel<glm::quat>
	{
	public:
		QuatChannel(std::vector<Keyframe<float>> x, std::vector<Keyframe<float>> y, std::vector<Keyframe<float>> z);
		virtual ~QuatChannel() = default;

		glm::quat Evaluate(float time) override;
	};

	class BoneChannel
	{
	public:
		BoneChannel(PamBoneAnim& pamBone);
		~BoneChannel() = default;
		BoneChannel(BoneChannel&& other) = default;

		BoneFrame Evaluate(float time);

	private:
		std::unique_ptr<VectorChannel> translate;
		std::unique_ptr<QuatChannel> rotate;
		std::unique_ptr<VectorChannel> scale;
	};

	class Anim
	{
	public:
		Anim(PamAnim& pamAnim);
		~Anim() = default;
		Anim(Anim&& other) = default;

		int BoneCount();
		int FrameCount();
		float GetTime();
		float GetFrame();

		void SetLoop(bool loop);
		void SetTime(float time);
		void SetFrame(float frame);
		
		void Update(float deltaTime);
		BoneFrame GetBone(int boneIdx);

	private:
		int frameRate, frameCount, loopFrom, loopTo;
		float currTime, currFrame;
		bool shouldLoop;
		std::vector<BoneChannel> bones;
	};
}