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