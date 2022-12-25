#pragma once
#include "Common.h"

class CBone
{
	int index;
	int parentIndex;
	std::string name;
	glm::mat4 transform;
	glm::mat4 inverseTransform;
};

class CSkeleton
{
public:
	std::vector<CBone> bones;
};

