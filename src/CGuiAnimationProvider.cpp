#include "CGuiAnimationProvider.h"
#include "glm/gtc/type_ptr.hpp"

Transform::Transform()
	: position(0.0f), rotation(0.0f), scale(1.0f)
{}

Transform::Transform(glm::vec3 p, glm::vec3 r, glm::vec3 s)
	: position(p), rotation(r), scale(s)
{}

glm::mat4 Transform::GetTransform()
{
	glm::quat q = glm::quat(rotation);
	glm::mat4 t = glm::mat4(1.0f);
	t = glm::translate(t, position);
	t = glm::rotate(t, glm::angle(q), glm::axis(q));
	t = glm::scale(t, scale);
	return t;
}

// TODO: Transform::SetTransform(glm::mat4 t)

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

glm::mat4 CGUIAnimationProvider::GetBone(int idx)
{
	return skeleton.bones[idx].transform * currTransforms[idx].GetTransform();
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