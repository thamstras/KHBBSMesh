#include "CAnimationDriver.h"
#include "glm/gtx/orthonormalize.hpp"

CAnimationDriver::CAnimationDriver(std::vector<AnimBone> bones)
{
	// copy bones
	m_bones = std::vector<AnimBone>(bones);

	Init();
}

CAnimationDriver::CAnimationDriver(const CSkeleton& skel)
{
	for (auto& b : skel.bones)
	{
		AnimBone ab = AnimBone();
		ab.idx = b.index;
		ab.parentIdx = b.parentIndex;
		ab.transform = b.transform;
		ab.inverseTransform = b.inverseTransform;
		m_bones.push_back(ab);
	}

	Init();
}

void CAnimationDriver::Init()
{
	// allocate vectors
	m_bindPose.reserve(m_bones.size());
	m_finalTransforms.reserve(m_bones.size());

	// Copy initial transforms to bind pose + initialize transforms
	for (auto& bone : m_bones)
	{
		m_bindPose.push_back(bone.transform);
		m_finalTransforms.push_back(glm::mat4(1.0f));
	}
}

void CAnimationDriver::Update(float deltaTime, double worldTime)
{
	if (m_currentAnim == nullptr)
		return;

	for (auto& bone : m_bones)
	{
		// TODO: I'm not convinced by the ordering of these transform multiplications...
		BoneFrame frameTransform = m_currentAnim->GetBone(bone.idx);
		glm::mat4 parentTransform = (bone.parentIdx == -1) ? glm::mat4(1.0f) : m_bones[bone.parentIdx].transform;
		
		/*if (m_currentAnim->NeedsScaleHack())
		{
			glm::vec4 scale;
			for (int i = 0; i < 3; i++) scale[i] = glm::length(glm::vec3(parentTransform[i]));
			scale[3] = 1.0f;
			parentTransform = glm::mat4(
				parentTransform[0] / scale[0],
				parentTransform[1] / scale[1],
				parentTransform[2] / scale[2],
				parentTransform[3] / scale[3]
			);
		}*/

		glm::mat4 globalTransform = parentTransform * frameTransform.fullTransform;

		//if (m_currentAnim->NeedsScaleHack())
		//	bone.transform = parentTransform * frameTransform.unscaledTransform;
		//else
		//	bone.transform = parentTransform * frameTransform.fullTransform;
		bone.transform = globalTransform;
		
		if (m_currentAnim->NeedsScaleHack())
		{
			glm::mat3 m = glm::mat3(globalTransform);
			m = glm::orthonormalize(m);
			globalTransform = glm::mat4(
				glm::vec4(m[0], globalTransform[0][3]),
				glm::vec4(m[1], globalTransform[1][3]),
				glm::vec4(m[2], globalTransform[2][3]),
				globalTransform[3]
			);
			globalTransform = glm::scale(globalTransform, frameTransform.scale);
		}

		glm::mat4 vertTransform = globalTransform * bone.inverseTransform;
		m_finalTransforms[bone.idx] = vertTransform;

	}

}

void CAnimationDriver::ResetPose()
{
	for (auto& bone : m_bones)
	{
		bone.transform = m_bindPose[bone.idx];
		m_finalTransforms[bone.idx] = glm::mat4(1.0f);
	}

}

void CAnimationDriver::SetAnimation(CAnimationProvider* newAnim)
{
	ResetPose();
	// TODO: Check bone count?
	m_currentAnim = newAnim;
}

std::vector<glm::mat4> CAnimationDriver::GetTransforms()
{
	return m_finalTransforms;
}

std::vector<AnimBone> CAnimationDriver::GetPose()
{
	return m_bones;
}