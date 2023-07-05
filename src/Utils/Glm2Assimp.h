#pragma once
#include "..\Common.h"
#include "assimp/matrix4x4.h"

class Glm2Assimp
{
public:
	static aiMatrix4x4 Matrix4(glm::mat4 m);
};