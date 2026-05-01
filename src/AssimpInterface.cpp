#include "AssimpInterface.h"

#include <assimp/Exporter.hpp>
#include <assimp/scene.h>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "Utils/Glm2Assimp.h"

aiSceneWrapper::aiSceneWrapper()
	: mMeshes(), mMaterials(), mAllNodes(), mRootNode(nullptr)
{ }

aiSceneWrapper::~aiSceneWrapper()
{
	mRootNode = nullptr;
	for (auto& itr : mAllNodes)
	{
		delete itr;
		itr = nullptr;
	}
	mAllNodes.clear();
	for (auto& itr : mMeshes)
	{
		delete itr;
		itr = nullptr;
	}
	mMeshes.clear();
	for (auto& itr : mMaterials)
	{
		delete itr;
		itr = nullptr;
	}
	mMaterials.clear();
}

aiScene* aiSceneWrapper::Finish()
{
	aiScene* scene = new aiScene();
	scene->mRootNode = mRootNode;
	scene->mNumMeshes = mMeshes.size();
	scene->mNumMaterials = mMaterials.size();
	scene->mNumAnimations = mAnimations.size();

	aiMesh** meshArr = new aiMesh * [scene->mNumMeshes];
	for (int i = 0; i < scene->mNumMeshes; i++) { meshArr[i] = mMeshes[i]; }
	scene->mMeshes = meshArr;

	aiMaterial** matArr = new aiMaterial * [scene->mNumMaterials];
	for (int i = 0; i < scene->mNumMaterials; i++) { matArr[i] = mMaterials[i]; }
	scene->mMaterials = matArr;

	aiAnimation** animArr = new aiAnimation * [scene->mNumAnimations];
	for (int i = 0; i < scene->mNumAnimations; i++) { animArr[i] = mAnimations[i]; }
	scene->mAnimations = animArr;

	// These belong to the actual aiScene now.
	mMeshes.clear();
	mMaterials.clear();
	mRootNode = nullptr;
	mAllNodes.clear();
	mAnimations.clear();

	return scene;
}

aiMeshWrapper::aiMeshWrapper()
	: mVertices(), mColors(), mFaces(), mTextureCoords()
{ 
	mMaterialIndex = 0;
}

aiMesh* aiMeshWrapper::Finish()
{
	aiMesh* mesh = new aiMesh();
	mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
	mesh->mMaterialIndex = mMaterialIndex;

	assert(mColors.size() == mVertices.size() || mColors.size() == 0);
	assert(mTextureCoords.size() == mVertices.size() || mTextureCoords.size() == 0);
	
	if (mVertices.size() > 0)
	{
		assert(mFaces.size() > 0);

		mesh->mNumVertices = mVertices.size();
		mesh->mVertices = new aiVector3D[mVertices.size()];
		std::copy(mVertices.begin(), mVertices.end(), mesh->mVertices);

		mesh->mNumFaces = mFaces.size();
		mesh->mFaces = new aiFace[mFaces.size()];
		std::copy(mFaces.begin(), mFaces.end(), mesh->mFaces);

		if (mColors.size() > 0)
		{
			mesh->mColors[0] = new aiColor4D[mVertices.size()];
			std::copy(mColors.begin(), mColors.end(), mesh->mColors[0]);
		}

		if (mTextureCoords.size() > 0)
		{
			mesh->mNumUVComponents[0] = 2;
			mesh->mTextureCoords[0] = new aiVector3D[mVertices.size()];
			std::copy(mTextureCoords.begin(), mTextureCoords.end(), mesh->mTextureCoords[0]);
		}
	}

	if (mBones.size() > 0)
	{
		mesh->mNumBones = mBones.size();
		mesh->mBones = new aiBone*[mesh->mNumBones];
		for (int i = 0; i < mesh->mNumBones; i++)
			mesh->mBones[i] = mBones[i].Finish();
	}

	return mesh;
}

void aiSceneWrapper::AddNodeToRoot(aiNode* node)
{
	mAllNodes.push_back(node);
	mRootNode->addChildren(1, &node);
	node->mParent = mRootNode;
}

ExportFormat::ExportFormat(const aiExportFormatDesc* desc)
{
	this->id = std::string(desc->id);
	this->desc = std::string(desc->description);
	this->ext = std::string(desc->fileExtension);
}

std::vector<ExportFormat> GetExportOptions()
{
	Assimp::Exporter exp = Assimp::Exporter();
	size_t formatCount = exp.GetExportFormatCount();
	std::vector<ExportFormat> formats = std::vector<ExportFormat>();
	for (int i = 0; i < formatCount; i++)
	{
		auto format = exp.GetExportFormatDescription(i);
		formats.push_back(ExportFormat(format));
	}

	return formats;
}

aiFace aiFaceWrapper::Finish()
{
	aiFace face = aiFace();
	face.mNumIndices = 3;
	face.mIndices = new unsigned int[3];
	std::copy(this->mIndices, this->mIndices + 4, face.mIndices);
	return face;
}

aiBoneWrapper::aiBoneWrapper()
	: mName(), mWeights(), mOffsetMatrix(1.0f)
{

}

aiBone* aiBoneWrapper::Finish()
{
	aiBone* bone = new aiBone();
	bone->mName.Set(mName);
	bone->mOffsetMatrix = Glm2Assimp::Matrix4(mOffsetMatrix);
	if (mWeights.size() > 0)
	{
		bone->mNumWeights = mWeights.size();
		bone->mWeights = new aiVertexWeight[mWeights.size()];
		std::copy(mWeights.begin(), mWeights.end(), bone->mWeights);
	}
	return bone;
}

aiNodeAnimWrapper::aiNodeAnimWrapper()
	: mNodeName(), mPositionKeys(), mRotationKeys(), mScalingKeys(), mPreState(aiAnimBehaviour_CONSTANT), mPostState(aiAnimBehaviour_CONSTANT)
{

}

aiNodeAnim* aiNodeAnimWrapper::Finish()
{
	for (int i = 1; i < mPositionKeys.size(); i++)
		assert(mPositionKeys[i].mTime > mPositionKeys[i - 1].mTime);
	for (int i = 1; i < mRotationKeys.size(); i++)
		assert(mRotationKeys[i].mTime > mRotationKeys[i - 1].mTime);
	for (int i = 1; i < mScalingKeys.size(); i++)
		assert(mScalingKeys[i].mTime > mScalingKeys[i - 1].mTime);
	aiNodeAnim* anim = new aiNodeAnim();
	anim->mNodeName.Set(mNodeName);
	anim->mNumPositionKeys = mPositionKeys.size();
	anim->mPositionKeys = new aiVectorKey[mPositionKeys.size()];
	std::copy(mPositionKeys.begin(), mPositionKeys.end(), anim->mPositionKeys);
	anim->mNumRotationKeys = mRotationKeys.size();
	anim->mRotationKeys = new aiQuatKey[mRotationKeys.size()];
	std::copy(mRotationKeys.begin(), mRotationKeys.end(), anim->mRotationKeys);
	anim->mNumScalingKeys = mScalingKeys.size();
	anim->mScalingKeys = new aiVectorKey[mScalingKeys.size()];
	std::copy(mScalingKeys.begin(), mScalingKeys.end(), anim->mScalingKeys);
	anim->mPreState = mPreState;
	anim->mPostState = mPostState;
	return anim;
}

aiNodeWrapper::aiNodeWrapper()
	: mName(), mTransformation(1.0f), mParent(nullptr), mChildren(), mMeshes()
{

}

void aiNodeWrapper::AddChild(aiNodeWrapper node)
{
	node.mParent = this;
	mChildren.push_back(node);
}

aiNode* aiNodeWrapper::FinishTree(aiNode* parent)
{
	aiNode* node = new aiNode();
	node->mName.Set(mName);
	node->mTransformation = Glm2Assimp::Matrix4(mTransformation);
	node->mParent = parent;
	if (mChildren.size() > 0)
	{
		node->mNumChildren = mChildren.size();
		node->mChildren = new aiNode*[node->mNumChildren];
		for (int c = 0; c < node->mNumChildren; c++)
			node->mChildren[c] = mChildren[c].FinishTree(node);
	}
	if (mMeshes.size() > 0)
	{
		node->mNumMeshes = mMeshes.size();
		node->mMeshes = new unsigned int[mMeshes.size()];
		std::copy(mMeshes.begin(), mMeshes.end(), node->mMeshes);
	}
	return node;
}

aiAnimationWrapper::aiAnimationWrapper()
	: mName(), mDuration(0.0), mTicksPerSecond(0.0), mChannels()
{

}

aiAnimation* aiAnimationWrapper::Finish()
{
	aiAnimation* anim = new	aiAnimation();
	anim->mName.Set(mName);
	anim->mDuration = mDuration;
	anim->mTicksPerSecond = mTicksPerSecond;
	anim->mNumChannels = mChannels.size();
	anim->mChannels = new aiNodeAnim*[anim->mNumChannels];
	for (int c = 0; c < anim->mNumChannels; c++)
		anim->mChannels[c] = mChannels[c].Finish();
	return anim;
}