#pragma once
#include "Common.h"
#include "CSkeleton.h"

// Polymorphic base class
class CAnimationProvider
{
public:
	CAnimationProvider() = default;
	virtual ~CAnimationProvider() {};
	virtual int BoneCount() = 0;
	virtual void Update(float deltaTime, double worldTime) = 0;
	virtual glm::mat4 GetBone(int idx) = 0;
	
	virtual void SetAnimTime(float time) = 0;
	virtual void SetPlayRate(float rate) = 0;
	virtual void SetPlaying(bool isPlaying) = 0;
};

struct AnimBone
{
	int idx;
	int parentIdx;
	
	// Global transform of this bone
	glm::mat4 transform;

	// Inverse of the transform of the bind pose
	glm::mat4 inverseTransform;
};

// TODO: Make this a base class (not needed for just a mesh viewer)
class CAnimationDriver
{
	std::vector<AnimBone> m_bones;
	std::vector<glm::mat4> m_bindPose;
	std::vector<glm::mat4> m_finalTransforms;
	CAnimationProvider* m_currentAnim;

public:
	// NOTE: The condition (parentIdx < idx) must hold for all bones.
	// IE: The skeleton must be sorted breadth first. I think this is true
	//     for BBS' skeletons but if we start importing external skeletons
	//     we'll have to check for it.

	CAnimationDriver(std::vector<AnimBone> bones);
	CAnimationDriver(const CSkeleton& skel);

	void Update(float deltaTime, double worldTime);

	void ResetPose();

	void SetAnimation(CAnimationProvider* newAnim);

	std::vector<glm::mat4> GetTransforms();
	std::vector<AnimBone> GetPose();

private:
	void Init();
};