#pragma once
#include "Common.h"
#include "Core/CAnimationDriver.h"
#include "FileTypes/BbsPam.h"

class AssimpAnimExporter;

namespace BBS
{
	struct Keyframe
	{
		int frameNumber;
		float value;
	};

	class IAnimChannel
	{
	public:
		IAnimChannel();
		virtual ~IAnimChannel() = default;

		virtual float Evaluate(int frame) = 0;
		virtual int KeyframeCount() = 0;
		virtual Keyframe GetKeyframe(int keyframeIdx) = 0;

		friend class ::AssimpAnimExporter;
	};

	class CConstChannel : public IAnimChannel
	{
	public:
		CConstChannel(float value);
		virtual ~CConstChannel() = default;

		virtual float Evaluate(int frame) override;
		virtual int KeyframeCount() override;
		virtual Keyframe GetKeyframe(int keyframeIdx) override;

	private:
		float theValue;

		friend class ::AssimpAnimExporter;
	};

	class CKeyframeChannel : public IAnimChannel
	{
	public:
		CKeyframeChannel(std::vector<Keyframe> keyframes);
		virtual ~CKeyframeChannel() = default;

		virtual float Evaluate(int frame) override;
		virtual int KeyframeCount() override;
		virtual Keyframe GetKeyframe(int keyframeIdx) override;
	private:
		std::vector<Keyframe> keyframes;
		int FindPrevKey(int frame);

		friend class ::AssimpAnimExporter;
	};

	class CBoneAnim
	{
	public:
		CBoneAnim(PamBoneAnim& pamBone);
		~CBoneAnim() = default;

		CBoneAnim(CBoneAnim&& other) = default;

		glm::mat4 Evaluate(int frame);

	private:
		std::unique_ptr<IAnimChannel> tx, ty, tz;
		std::unique_ptr<IAnimChannel> rx, ry, rz;
		std::unique_ptr<IAnimChannel> sx, sy, sz;

		std::unique_ptr<IAnimChannel> MakeChannel(std::optional<PamAnimChannel>& channel, float defaultValue = 0.0f);

		friend class ::AssimpAnimExporter;
	};

	class CBBSAnim
	{
	public:
		CBBSAnim(PamAnim& pamAnim);
		~CBBSAnim() = default;

		CBBSAnim(CBBSAnim&& other) = default;
		
		void Update(float animTime);
		void SetTime(float time);
		glm::mat4 GetBone(int boneIdx);

	private:
		void CalcFrame();

		int frameRate, frameCount, loopFrom, loopTo, boneCount;
		bool shouldLoop = true;
		float currTime = 0.0f;
		int currFrame = 0;
		std::vector<CBoneAnim> bones;

		friend class CBBSAnimSet;
		friend class ::AssimpAnimExporter;
	};

	/* NOTE: In an actual engine AnimSet would just be storage and we'd plug the anims straight into 
			 some kind of specialized AnimationDriver and let it handle anim selection/blending etc.
			 But this is just a simple anim viewer so we'll hide away anim selection here.
	*/
	class CBBSAnimSet : public CAnimationProvider
	{
		struct AnimInfo
		{
			std::string name;
			int idx;
		};

	public:
		CBBSAnimSet(PamFile& pamFile, CSkeleton skel);
		~CBBSAnimSet();

		virtual int BoneCount() override;
		virtual void Update(float deltaTime, double worldTime) override;
		virtual glm::mat4 GetBone(int idx) override;

		virtual void SetAnimTime(float time) override;
		virtual void SetPlayRate(float rate) override;
		virtual void SetPlaying(bool isPlaying) override;
		virtual bool NeedsScaleHack() override;

		int AnimCount();
		void SelectAnim(int idx);

		// TODO: GUI Interface!!!
		void GUI_DrawControls();

	private:
		float timescale = 1.0f;
		float currAnimTime = 0.0f;
		bool isPlaying = false;
		int selectedIdx = 0;
		std::vector<CBBSAnim> anims;
		std::vector<AnimInfo> animInfos;
		CSkeleton skeleton;
		bool gui_showPose = false;

		friend class ::AssimpAnimExporter;
	};
}