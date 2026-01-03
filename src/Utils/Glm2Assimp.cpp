#include "Glm2Assimp.h"

#include "assimp/matrix4x4.inl"

aiMatrix4x4 Glm2Assimp::Matrix4(glm::mat4 m)
{
	m = glm::transpose(m);
	return aiMatrix4x4(m[0][0], m[0][1], m[0][2], m[0][3],
		               m[1][0], m[1][1], m[1][2], m[1][3],
		               m[2][0], m[2][1], m[2][2], m[2][3], 
		               m[3][0], m[3][1], m[3][2], m[3][3]);
}

aiVector3D Glm2Assimp::Vector3(glm::vec3 v)
{
	return aiVector3D(v.x, v.y, v.z);
}

aiQuaternion Glm2Assimp::Quat(glm::quat q)
{
	return aiQuaternion(q.w, q.x, q.y, q.z);
}

aiColor4D Glm2Assimp::Color(glm::vec4 c)
{
	return aiColor4D(c.r, c.g, c.b, c.a);
}