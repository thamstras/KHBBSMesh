#include "CGuiAnimationProvider.h"
#include "glm/gtc/type_ptr.hpp"


CGUIAnimationProvider::CGUIAnimationProvider(CSkeleton skel)
	: skeleton(skel)
{
	for (int i = 0; i < skeleton.bones.size(); i++)
		currTransforms.push_back(Transform());
}

CGUIAnimationProvider::~CGUIAnimationProvider() {}

int CGUIAnimationProvider::BoneCount()
{
	return skeleton.bones.size();
}

void CGUIAnimationProvider::Update(float delta, double world)
{
	// NOP
}

BoneFrame CGUIAnimationProvider::GetBone(int idx)
{
	glm::mat4 mat = skeleton.bones[idx].transform * currTransforms[idx].GetTransform();
	return { mat, mat };
}

void CGUIAnimationProvider::SetAnimTime(float time)
{
	// NOP
}

void CGUIAnimationProvider::SetPlayRate(float rate)
{
	// NOP
}

void CGUIAnimationProvider::SetPlaying(bool isPlaying)
{
	// NOP
}

bool CGUIAnimationProvider::NeedsScaleHack()
{
	return false;
}

void CGUIAnimationProvider::GUI_DrawControls()
{
	for (int i = 0; i < currTransforms.size(); i++)
	{
		CBone& bone = skeleton.bones[i];
		Transform& t = currTransforms[i];
		ImGui::PushID(i);
		ImGui::Text("Bone: %s", bone.name.c_str());
		ImGui::DragFloat3("Position", glm::value_ptr(t.position), 0.1f);
		ImGui::DragFloat3("Rotation", glm::value_ptr(t.rotation), 0.1f, -180.0f, 180.0f);
		ImGui::DragFloat3("Scale", glm::value_ptr(t.scale), 0.1f);
		ImGui::Separator();
		ImGui::PopID();
	}
}