#pragma once
#include "Common.h"
#include "Core\CTexture.h"
#include "Core\CMesh.h"
#include <map>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

class aiFaceWrapper
{
public:
	unsigned int mIndices[3] = { 0 };

	aiFace Finish();
};

class aiBoneWrapper
{
public:
	aiBoneWrapper();

	std::string mName;
	std::vector<aiVertexWeight> mWeights;
	glm::mat4 mOffsetMatrix;

	aiBone* Finish();
};

class aiMeshWrapper
{
public:
	aiMeshWrapper();

	std::vector<aiVector3D> mVertices;
	std::vector<aiColor4D> mColors;
	std::vector<aiFace> mFaces;
	std::vector<aiVector3D> mTextureCoords;
	std::vector<aiBoneWrapper> mBones;
	unsigned int mMaterialIndex;

	aiMesh* Finish();
};

class aiNodeAnimWrapper
{
public:
	aiNodeAnimWrapper();

	std::string mNodeName;
	std::vector<aiVectorKey> mPositionKeys;
	std::vector<aiQuatKey> mRotationKeys;
	std::vector<aiVectorKey> mScalingKeys;
	aiAnimBehaviour mPreState;
	aiAnimBehaviour mPostState;

	aiNodeAnim* Finish();
};

class aiNodeWrapper
{
public:
	aiNodeWrapper();

	std::string mName;
	glm::mat4 mTransformation;
	aiNodeWrapper* mParent;
	std::vector<aiNodeWrapper> mChildren;
	std::vector<unsigned int> mMeshes;

	void AddChild(aiNodeWrapper node);
	aiNode* FinishTree(aiNode* parent = nullptr);
};

class aiAnimationWrapper
{
public:
	aiAnimationWrapper();

	std::string mName;
	double mDuration;
	double mTicksPerSecond;
	std::vector<aiNodeAnimWrapper> mChannels;

	aiAnimation* Finish();
};

class aiSceneWrapper
{
public:
	aiSceneWrapper();
	~aiSceneWrapper();

	std::vector<aiMesh*> mMeshes;
	std::vector<aiMaterial*> mMaterials;

	aiNode* mRootNode;
	std::vector<aiNode*> mAllNodes;

	std::vector<aiAnimation*> mAnimations;

	aiScene* Finish();
	void AddNodeToRoot(aiNode* node);
};

class AssimpExporter
{
public:
	AssimpExporter();
	~AssimpExporter();

	void BeginExport();

	void AddTexture(std::string name, CTexture* texture);

	void AddMesh(CMesh* mesh);

	void EndExport(std::string folderPath, std::string mapname);

private:
	// Call AddMesh instead
	unsigned int AddSection(CMeshSection* section, CMesh* parent);

	std::map<std::string, int> textureMap;
	std::map<int, std::string> textureUnmap;
	std::vector<CTexture*> textureList;

	aiSceneWrapper* scene;
};

struct ExportFormat
{
	std::string id;
	std::string desc;
	std::string ext;

	ExportFormat(const aiExportFormatDesc* desc);
};

std::vector<ExportFormat> GetExportOptions();