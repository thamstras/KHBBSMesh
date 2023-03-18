#pragma once
#include "Common.h"

struct Transform
{
public:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	Transform(glm::vec3 p, glm::vec3 r, glm::vec3 s);
	Transform();
	glm::mat4 GetTransform();
	void SetTransform(glm::mat4 transform);

	static Transform Decompose(glm::mat4 t);
};