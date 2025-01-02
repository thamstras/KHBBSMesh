#pragma once
#include "..\Common.h"
#include "..\Core\CMesh.h"
#include "..\Core\CTexture.h"
#include "CoreRender.h"
#include "CAnimationDriver.h"

class CSkelMesh;

struct CSkelMeshSection
{
	unsigned int vertCount;
	unsigned int textureIndex;
	unsigned int stride;
	GLenum primType;
	bool twoSided;
	bool blend;
	std::vector<unsigned int> kickList;
	uint8_t hideGroup;
};

class CSkelMesh
{
private:
	static CTexture* dummyTexture;
	static glm::vec2 zeroOffset;

	std::shared_ptr<CShader> SelectShader(RenderContext& context);

public:
	CSkelMesh();
	~CSkelMesh();
	void Build();
	void UnBuild();

	std::vector<CSkelMeshSection> sections;
	std::vector<CTexture*> textures;
	std::vector<glm::vec2> uvOffsets;
	std::vector<float> vertData;
	//std::vector<VERTEX_ATTRIBUTE> vertAttribs;
	unsigned int vertCount;
	unsigned int polyCount;

	glm::vec3 Pos, Rot, Scale;

	GLuint VAO, VBO;

	CAnimationDriver* driver;

	void Draw(RenderContext& context);
	void Draw(RenderContext& context, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
};