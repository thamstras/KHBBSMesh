#pragma once
#include "..\Common.h"
#include "assimp/matrix4x4.h"
#include "assimp/quaternion.h"
#include "assimp/color4.h"

class Glm2Assimp
{
public:
	static aiMatrix4x4 Matrix4(glm::mat4 m);
	static aiVector3D Vector3(glm::vec3 v);
	static aiQuaternion Quat(glm::quat q);
	static aiColor4D Color(glm::vec4 c);
};