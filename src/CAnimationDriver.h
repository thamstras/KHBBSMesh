#pragma once
#include "Common.h"

// TODO: Move to core
// TODO: Only a sketch. May need to change once I write CBBSAnimationProvider and/or CSkelMesh

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
	// TODO: Is this the transform or the inverse transform?
	// If it's the inverse transform, do we need to invert the anim transforms in Update?
	glm::mat4 transform;
};

class CAnimationDriver
{
	std::vector<AnimBone> m_bones;
	std::vector<glm::mat4> m_bindPose;
	std::vector<glm::mat4> m_accumulatedTransforms;
	CAnimationProvider* m_currentAnim;

public:
	// NOTE: The condition (parentIdx < idx) must hold for all bones.
	// IE: The skeleton must be sorted breadth first. I think this is true
	//     for BBS' skeletons but if we start importing external skeletons
	//     we'll have to check for it.

	CAnimationDriver(std::vector<AnimBone> bones)
	{
		// copy bones
		m_bones = std::vector<AnimBone>(bones);

		// allocate vectors
		m_bindPose.reserve(m_bones.size());
		m_accumulatedTransforms.reserve(m_bones.size());

		// Copy initial transforms to bind pose + build initial transforms array
		for (auto& bone : m_bones)
		{
			m_bindPose.push_back(bone.transform);

			glm::mat4 accTransform;
			
			if (bone.parentIdx == -1)
				accTransform = bone.transform;
			else
				accTransform = bone.transform * m_accumulatedTransforms[bone.parentIdx];

			m_accumulatedTransforms.push_back(accTransform);
		}
	}

	void Update(float deltaTime, double worldTime)
	{
		if (m_currentAnim == nullptr)
			return;
		
		// TODO: Do we call this, or is whoever owns the animation provider responsible?
		//       If we're responsible, we'll need stop/start/reset functions
		m_currentAnim->Update(deltaTime, worldTime);

		for (auto& bone : m_bones)
		{
			glm::mat4 frameTransform = m_currentAnim->GetBone(bone.idx);

			// Don't know if anim transform will be delta from last frame or delta from bind pose.
			// # IF (delta from last frame)
			bone.transform = frameTransform * bone.transform;

			// # IF (delta from bind pose)
			bone.transform = frameTransform * m_bindPose[bone.idx];

			if (bone.parentIdx == -1)
				m_accumulatedTransforms[bone.idx] = bone.transform;
			else
				m_accumulatedTransforms[bone.idx] = bone.transform * m_accumulatedTransforms[bone.parentIdx];
		}

		// Then m_accumulatedTransforms gets passed to the mesh renderer to transform the actual verts
		// I THINK. I COULD BE WRONG. We'll see once I write CSkelMesh
	}

	void ResetPose()
	{
		for (auto& bone : m_bones)
			bone.transform = m_bindPose[bone.idx];
	}

	void SetAnimation(CAnimationProvider* newAnim)
	{
		ResetPose();
		// TODO: Check bone count?
		m_currentAnim = newAnim;
	}
};