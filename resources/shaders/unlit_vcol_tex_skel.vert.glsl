#version 330

const int MAX_BONE_INFLUENCE = 8;
const int MAX_BONES = 200;

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex;
// TODO: In BBS the bone IDXs are uniform across each draw call
// Is it worth keeping this optimization, or staying with this more
// general approach?
layout(location = 3) in ivec4 boneIdxLo;
layout(location = 4) in ivec4 boneIdxHi;
layout(location = 5) in vec4 boneWeightLo;
layout(location = 6) in vec4 boneWeightHi;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 boneTransforms[MAX_BONES];

out vec4 vColor;
out vec2 vTex;
out float vDepth;

// NOTE: No range checks, don't fuck up.
int GetBoneIdx(int boneNum)
{
	if (boneNum < 4) return boneIdxLo[boneNum];
	else return boneIdxHi[boneNum - 4];
}

float GetBoneWeight(int boneNum)
{
	if (boneNum < 4) return boneWeightLo[boneNum];
	else return boneWeightHi[boneNum - 4];
}

//int GetBoneTransform(int boneIdx)
//{
//	if (boneIdx < 0) return mat4(1.0);
//	if (boneIdx < MAX_BONES) return boneTransforms[boneIdx];
//	return mat4(1.0);
//}

void main()
{
	vec3 pos3 = vec3(position);
	vec4 blendedPosition = vec4(0.0);
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		int boneIdx = GetBoneIdx(i);
		if (boneIdx == -1 || boneIdx >= MAX_BONES)
			continue;

		mat4 boneTransform = boneTransforms[boneIdx];
		vec4 bonePosition = boneTransform * vec4(pos3, 1.0);
		blendedPosition += bonePosition * GetBoneWeight(i);
	}

	gl_Position = projection * view * model * blendedPosition;
	vColor = color;
	vTex = tex;
	vDepth = -(view * model * position).z;
}

// TODO: For now we'll reuse unlit_vcol_tex.frag.glsl but I don't think skeletal meshes need depth fog?