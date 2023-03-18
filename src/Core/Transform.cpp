#include "Core/Transform.h"

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

void Transform::SetTransform(glm::mat4 t)
{
	position = t[3];
	for (int i = 0; i < 3; i++)
		scale[i] = glm::length(glm::vec3(t[i]));
	const glm::mat3 rotMtx(
		glm::vec3(t[0]) / scale[0],
		glm::vec3(t[1]) / scale[1],
		glm::vec3(t[2]) / scale[2]);
	glm::quat rot = glm::quat_cast(rotMtx);
	rotation = glm::eulerAngles(rot);
}

Transform Transform::Decompose(glm::mat4 t)
{
	Transform result = Transform();
	result.SetTransform(t);
	return result;
}