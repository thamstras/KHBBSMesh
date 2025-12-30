#pragma once
#include "Common.h"
#include "Core/CAnimationDriver.h"
#include "Core/Transform.h"
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

		int KeyframeCount() const
		{
			return keyframes.size();
		}

		Keyframe<T> GetKeyframe(int idx) const
		{
			return keyframes[idx];
		}

	protected:
		std::vector<Keyframe<T>> keyframes;
		int FindPrevKeyframeIdx(float t) const
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

		BoneFrame Evaluate(float time) const;
		Transform GetTransform(float time) const;

		VectorChannel const* GetRawTranslate() const;
		QuatChannel const* GetRawRotate() const;
		VectorChannel const* GetRawScale() const;
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

		int BoneCount() const;
		int FrameRate() const;
		int FrameCount() const;
		int LoopFrom() const;
		int LoopTo() const;
		
		void ModFrameRate(int newFrameRate);
		void ModLoopFrom(int newLoopFrom);
		void ModLoopTo(int newLoopTo);
		
		BoneFrame GetBone(float fTime, int boneIdx);
		BoneChannel const* GetRaw(int boneIdx) const;

	private:
		int frameRate, frameCount, loopFrom, loopTo;
		std::vector<BoneChannel> bones;
	};

	class CBBSAnimationProvider : public CAnimationProvider
	{
		struct AnimInfo
		{
			std::string name;
			int storeIdx = -1;
		};

	public:
		CBBSAnimationProvider(PamFile& file, CSkeleton skel);
		virtual ~CBBSAnimationProvider();

		virtual int BoneCount() override;
		virtual void Update(float deltaTime, double worldTime) override;
		virtual BoneFrame GetBone(int idx) override;

		virtual void SetAnimTime(float time) override;
		virtual void SetPlayRate(float rate) override;
		virtual void SetPlaying(bool isPlaying) override;
		virtual bool NeedsScaleHack() override;
		
		int AnimCount() const;
		void SelectAnim(int idx);

		void SetAnimFrame(float frame);

		void GUI_Controls();
	
	private:
		float currTime = 0.0f, currFrame = 0.0f, playbackRate = 1.0f;
		bool isPlaying = false, shouldLoop = false;
		void CalcFrame();

		int selectedIdx = 0;
		Anim* selectedAnim = nullptr;
		std::vector<AnimInfo> animInfos;
		std::vector<Anim> anims;

		CSkeleton skeleton;
		bool gui_showPose = false;
	};
}