#include "CAnimationDriver.h"

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

	// TODO: Do we call this, or is whoever owns the animation provider responsible?
	//       If we're responsible, we'll need stop/start/reset functions
	// UPDATE: It'll have to be whatever owns the animation provider.
	//m_currentAnim->Update(deltaTime, worldTime);

	for (auto& bone : m_bones)
	{
		// TODO: I'm not convinced by the ordering of these transform multiplications...
		glm::mat4 frameTransform = m_currentAnim->GetBone(bone.idx);
		glm::mat4 parentTransform = (bone.parentIdx == -1) ? glm::mat4(1.0f) : m_bones[bone.parentIdx].transform;
		glm::mat4 globalTransform = parentTransform * frameTransform;
		glm::mat4 vertTransform = globalTransform * bone.inverseTransform;

		bone.transform = globalTransform;
		m_finalTransforms[bone.idx] = vertTransform;

		/* OLD
		// Don't know if anim transform will be delta from last frame or delta from bind pose.
		// # IF (delta from last frame)
		bone.transform = frameTransform * bone.transform;

		// # IF (delta from bind pose)
		bone.transform = frameTransform * m_bindPose[bone.idx];

		if (bone.parentIdx == -1)
			m_accumulatedTransforms[bone.idx] = bone.transform;
		else
			m_accumulatedTransforms[bone.idx] = bone.transform * m_accumulatedTransforms[bone.parentIdx];
		*/
	}

	// Then m_accumulatedTransforms gets passed to the mesh renderer to transform the actual verts
	// I THINK. I COULD BE WRONG. We'll see once I write CSkelMesh
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