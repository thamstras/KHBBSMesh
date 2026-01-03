#include "CGuiAnimationProvider.h"
#include "glm/gtc/type_ptr.hpp"


CGUIAnimationProvider::CGUIAnimationProvider(CSkeleton skel)
	: skeleton(skel)
{
	for (int i = 0; i < skeleton.bones.size(); i++)
		currTransforms.push_back(Transform());
	Reset();
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
	//glm::mat4 mat = skeleton.bones[idx].transform * currTransforms[idx].GetTransform();
	glm::mat4 mat = currTransforms[idx].GetTransform();
	glm::vec3 s = currTransforms[idx].scale;
	currTransforms[idx].scale = glm::vec3(1.0f);
	glm::mat4 mat2 = currTransforms[idx].GetTransform();
	currTransforms[idx].scale = s;
	return { mat, mat2, s };
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
	return true;
}

void CGUIAnimationProvider::GUI_DrawControls()
{
	if (ImGui::Button("Reset Pose"))
		Reset();
	ImGui::Checkbox("Lock Scale", &gui_lockScale);
	for (int i = 0; i < currTransforms.size(); i++)
	{
		CBone& bone = skeleton.bones[i];
		Transform& t = currTransforms[i];
		ImGui::PushID(i);
		ImGui::Text("Bone: %s", bone.name.c_str());
		ImGui::DragFloat3("Position", glm::value_ptr(t.position), 0.1f);
		ImGui::DragFloat3("Rotation", glm::value_ptr(t.rotation), 0.01f, -glm::pi<float>(), glm::pi<float>());
		if (!gui_lockScale)
		{
			ImGui::DragFloat3("Scale", glm::value_ptr(t.scale), 0.1f);
		}
		else
		{
			float f = t.scale.x;
			if (ImGui::DragFloat("Scale", &f, 0.1f))
				t.scale = glm::vec3(f);
		}
		ImGui::Separator();
		ImGui::PopID();
	}
}

void CGUIAnimationProvider::Reset()
{
	for (int i = 0; i < skeleton.bones.size(); i++)
	{
		currTransforms[i].SetTransform(skeleton.bones[i].transform);
	}
}