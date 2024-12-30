#include "AssimpExporter.h"
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Utils/Glm2Assimp.h"
#include <filesystem>
#include "Utils/stb_image_write.h"
#include <algorithm>
#include "Utils/BitCast.h"

aiAnimation* AssimpAnimExporter::Export(BBS::CBBSAnim& anim, std::string name, CSkeleton& skel)
{
	aiAnimationWrapper outAnim = aiAnimationWrapper();
	outAnim.mName = name;
	outAnim.mDuration = anim.frameCount * 24.0f;
	outAnim.mTicksPerSecond = 24.0f;
	for (int i = 0; i < anim.boneCount; i++)
	{
		outAnim.mChannels.push_back(Export(anim.bones[i], skel.bones[i].name, anim));
	}
	return outAnim.Finish();
}

aiNodeAnimWrapper AssimpAnimExporter::Export(BBS::CBoneAnim& boneAnim, std::string boneName, BBS::CBBSAnim& parentAnim)
{
	// info
	aiNodeAnimWrapper outAnim = aiNodeAnimWrapper();
	outAnim.mNodeName = boneName;
	outAnim.mPreState = aiAnimBehaviour_CONSTANT;
	outAnim.mPostState = aiAnimBehaviour_CONSTANT;

	// Position
	int kcx, kcy, kcz;
	kcx = boneAnim.tx->KeyframeCount();
	kcy = boneAnim.ty->KeyframeCount();
	kcz = boneAnim.tz->KeyframeCount();
	int numPositionKeys = std::max(std::max(kcx, kcy), kcz);
	if (numPositionKeys == 1)
	{
		// all constants
		outAnim.mPositionKeys.push_back(aiVectorKey(0.0, aiVector3D(boneAnim.tx->Evaluate(0), boneAnim.ty->Evaluate(0), boneAnim.tz->Evaluate(0))));
		outAnim.mPositionKeys.push_back(aiVectorKey(parentAnim.frameCount / 24.0f, aiVector3D(boneAnim.tx->Evaluate(0), boneAnim.ty->Evaluate(0), boneAnim.tz->Evaluate(0))));
	}
	else if (numPositionKeys == parentAnim.frameCount)
	{
		// every frame
		for (int f = 0; f < numPositionKeys; f++)
		{
			float t = f / 24.0f;
			//float t = (float)f;
			outAnim.mPositionKeys.push_back(aiVectorKey(t, aiVector3D(boneAnim.tx->Evaluate(f), boneAnim.ty->Evaluate(f), boneAnim.tz->Evaluate(f))));
		}
	}
	else
	{
		outAnim.mPositionKeys = ExportVectorChannel(boneAnim.tx.get(), boneAnim.ty.get(), boneAnim.tz.get(), parentAnim.frameRate, parentAnim.frameCount);
	}

	// Rotation
	kcx = boneAnim.rx->KeyframeCount();
	kcy = boneAnim.ry->KeyframeCount();
	kcz = boneAnim.rz->KeyframeCount();
	int numRotationKeys = std::max(std::max(kcx, kcy), kcz);
	if (numRotationKeys == 1)
	{
		// all constants
		outAnim.mRotationKeys.push_back(aiQuatKey(0.0, aiQuaternion(boneAnim.ry->Evaluate(0), boneAnim.rz->Evaluate(0), boneAnim.rx->Evaluate(0))));
		outAnim.mRotationKeys.push_back(aiQuatKey(parentAnim.frameCount / 24.0f, aiQuaternion(boneAnim.ry->Evaluate(0), boneAnim.rz->Evaluate(0), boneAnim.rx->Evaluate(0))));
	}
	else if (numRotationKeys == parentAnim.frameCount)
	{
		// every frame
		for (int f = 0; f < numRotationKeys; f++)
		{
			float t = f / 24.0f;
			//float t = (float)f;
			outAnim.mRotationKeys.push_back(aiQuatKey(t, aiQuaternion(boneAnim.ry->Evaluate(f), boneAnim.rz->Evaluate(f), boneAnim.rx->Evaluate(f))));
		}
	}
	else
	{
		outAnim.mRotationKeys = ExportQuatChannel(boneAnim.rx.get(), boneAnim.ry.get(), boneAnim.rz.get(), parentAnim.frameRate, parentAnim.frameCount);
	}

	// Scale
	kcx = boneAnim.sx->KeyframeCount();
	kcy = boneAnim.sy->KeyframeCount();
	kcz = boneAnim.sz->KeyframeCount();
	int numScalingKeys = std::max(std::max(kcx, kcy), kcz);
	if (numScalingKeys == 1)
	{
		// all constants
		outAnim.mScalingKeys.push_back(aiVectorKey(0.0, aiVector3D(boneAnim.sx->Evaluate(0), boneAnim.sy->Evaluate(0), boneAnim.sz->Evaluate(0))));
		outAnim.mScalingKeys.push_back(aiVectorKey(parentAnim.frameCount / 24.0f, aiVector3D(boneAnim.sx->Evaluate(0), boneAnim.sy->Evaluate(0), boneAnim.sz->Evaluate(0))));
	}
	else if (numScalingKeys == parentAnim.frameCount)
	{
		// every frame
		for (int f = 0; f < numScalingKeys; f++)
		{
			float t = f / 24.0f;
			//float t = (float)f;
			outAnim.mScalingKeys.push_back(aiVectorKey(t, aiVector3D(boneAnim.sx->Evaluate(f), boneAnim.sy->Evaluate(f), boneAnim.sz->Evaluate(f))));
		}
	}
	else
	{
		outAnim.mScalingKeys = ExportVectorChannel(boneAnim.sx.get(), boneAnim.sy.get(), boneAnim.sz.get(), parentAnim.frameRate, parentAnim.frameCount);
	}

	return outAnim;
}

std::vector<aiVectorKey> AssimpAnimExporter::ExportVectorChannel(BBS::IAnimChannel* x, BBS::IAnimChannel* y, BBS::IAnimChannel* z, int frameRate, int frameCount)
{
	// the difficult one. - We know we need at least N keys, and there'll be gaps, but there's no gurantee the gaps in x/y/z line up,
	// so we might need more, and we'll have to step through the 3 separate channels taking the least remaining key each time.
	std::vector<aiVectorKey> keyList = std::vector<aiVectorKey>();
	
	int currFrame = -1, lastFrame;
	int idxs[3] = { 0 };
	int maxes[3] = {
		x->KeyframeCount() - 1,
		y->KeyframeCount() - 1,
		z->KeyframeCount() - 1
	};

	do
	{
		lastFrame = currFrame;

		BBS::Keyframe next[3] = {
			x->GetKeyframe(idxs[0]),
			y->GetKeyframe(idxs[1]),
			z->GetKeyframe(idxs[2])
		};

		if (next[0].frameNumber <= next[1].frameNumber && next[0].frameNumber <= next[2].frameNumber)
		{
			currFrame = next[0].frameNumber;
		}
		else if (next[1].frameNumber <= next[0].frameNumber && next[1].frameNumber <= next[2].frameNumber)
		{
			currFrame = next[1].frameNumber;
		}
		else
		{
			currFrame = next[2].frameNumber;
		}

		if (next[0].frameNumber == currFrame) idxs[0] = std::min(idxs[0] + 1, maxes[0]);
		if (next[1].frameNumber == currFrame) idxs[1] = std::min(idxs[1] + 1, maxes[1]);
		if (next[2].frameNumber == currFrame) idxs[2] = std::min(idxs[2] + 1, maxes[2]);

		if (currFrame != lastFrame)
		{
			keyList.emplace_back((float)currFrame / 24.0f, aiVector3D(x->Evaluate(currFrame), y->Evaluate(currFrame), z->Evaluate(currFrame)));
		}
	}
	while (currFrame != lastFrame && currFrame < frameCount);

	return keyList;
}

std::vector<aiQuatKey> AssimpAnimExporter::ExportQuatChannel(BBS::IAnimChannel* x, BBS::IAnimChannel* y, BBS::IAnimChannel* z, int frameRate, int frameCount)
{
	// the difficult one. - We know we need at least N keys, and there'll be gaps, but there's no gurantee the gaps in x/y/z line up,
	// so we might need more, and we'll have to step through the 3 separate channels taking the least remaining key each time.
	std::vector<aiQuatKey> keyList = std::vector<aiQuatKey>();

	int currFrame = -1, lastFrame;
	int idxs[3] = { 0 };
	int maxes[3] = {
		x->KeyframeCount() - 1,
		y->KeyframeCount() - 1,
		z->KeyframeCount() - 1
	};

	do
	{
		lastFrame = currFrame;

		BBS::Keyframe next[3] = {
			x->GetKeyframe(idxs[0]),
			y->GetKeyframe(idxs[1]),
			z->GetKeyframe(idxs[2])
		};

		if (next[0].frameNumber <= next[1].frameNumber && next[0].frameNumber <= next[2].frameNumber)
		{
			currFrame = next[0].frameNumber;
		}
		else if (next[1].frameNumber <= next[0].frameNumber && next[1].frameNumber <= next[2].frameNumber)
		{
			currFrame = next[1].frameNumber;
		}
		else
		{
			currFrame = next[2].frameNumber;
		}

		if (next[0].frameNumber == currFrame) idxs[0] = std::min(idxs[0] + 1, maxes[0]);
		if (next[1].frameNumber == currFrame) idxs[1] = std::min(idxs[1] + 1, maxes[1]);
		if (next[2].frameNumber == currFrame) idxs[2] = std::min(idxs[2] + 1, maxes[2]);

		if (currFrame != lastFrame)
		{
			keyList.emplace_back((float)currFrame / 24.0f, aiQuaternion(y->Evaluate(currFrame), z->Evaluate(currFrame), x->Evaluate(currFrame)));
		}
	} while (currFrame != lastFrame && currFrame < frameCount);

	return keyList;
}

aiNode* AssimpAnimExporter::Export(CSkeleton& skeleton)
{
	return ExportSubTree(skeleton.bones[0], skeleton, nullptr);
}

aiNode* AssimpAnimExporter::ExportSubTree(CBone& bone, CSkeleton& skeleton, aiNode* parent)
{
	aiNode* node = new aiNode();
	node->mName.Set(bone.name);
	node->mTransformation = Glm2Assimp::Matrix4(bone.transform);
	node->mParent = parent;
	std::vector<aiNode*> children = std::vector<aiNode*>();
	for (CBone& b : skeleton.bones)
	{
		if (b.parentIndex == bone.index)
			children.push_back(ExportSubTree(b, skeleton, node));
	}
	if (children.size() > 0)
		node->addChildren(children.size(), children.data());
	return node;
}

aiBoneWrapper AssimpAnimExporter::ExportBone(CBone& bone)
{
	aiBoneWrapper outBone = aiBoneWrapper();
	outBone.mName = bone.name;
	outBone.mOffsetMatrix = bone.inverseTransform;
	// NOTE: The actual binding of verts to bones will be done elsewhere
	return outBone;
}

// TODO: This should probably be part of aiMeshWrapper
void AddSection(aiMeshWrapper& mesh, CSkelMeshSection& section, float* vertData)
{
	int fptr = 0;
	int faceCount;
	
	int vertBase = mesh.mVertices.size();
	for (int vert = 0; vert < section.vertCount; vert++)
	{
		fptr = vert * 26;
		int idx[8]{};
		float pos[4]{}, col[4]{}, tex[2]{}, wei[8]{};
		for (int i = 0; i < 8; i++) idx[i] = bit_cast<int, float>(vertData[fptr++]);
		for (int i = 0; i < 8; i++) wei[i] = vertData[fptr++];
		for (int i = 0; i < 2; i++) tex[i] = vertData[fptr++];
		for (int i = 0; i < 4; i++) col[i] = vertData[fptr++];
		for (int i = 0; i < 4; i++) pos[i] = vertData[fptr++];
		mesh.mVertices.push_back(aiVector3D(pos[0], pos[1], pos[2]));
		mesh.mColors.push_back(aiColor4D(col[0] / 2.0f, col[1] / 2.0f, col[2] / 2.0f, col[3] / 2.0f));
		mesh.mTextureCoords.push_back(aiVector3D(tex[0], tex[1], 0.0f));
		for (int i = 0; i < 8; i++)
		{
			if (idx[i] >= 255) continue;
			if (wei[i] <= 0.0f) continue;
			mesh.mBones[idx[i]].mWeights.emplace_back(vertBase + vert, wei[i]);
		}
	}

	if (section.primType == GL_TRIANGLES)
	{
		faceCount = section.vertCount / 3;
		for (int f = 0; f < faceCount; f++)
		{
			aiFace face;
			face.mNumIndices = 3;
			face.mIndices = new unsigned int[3];
			face.mIndices[0] = vertBase + (f * 3) + 0;
			face.mIndices[1] = vertBase + (f * 3) + 1;
			face.mIndices[2] = vertBase + (f * 3) + 2;
			mesh.mFaces.push_back(face);
		}
	}
	else
	{
		assert(section.primType == GL_TRIANGLE_STRIP);
		int fBase = 0;
		for (unsigned int stripLen : section.kickList)
		{
			if (stripLen == 0)
				continue;

			assert(stripLen >= 3);
			
			for (int f = 0; f < stripLen - 2; f++)
			{
				aiFace face;
				face.mNumIndices = 3;
				face.mIndices = new unsigned int[3];
				if (f % 2 == 0)
				{
					face.mIndices[0] = vertBase + fBase + f + 0;
					face.mIndices[1] = vertBase + fBase + f + 1;
					face.mIndices[2] = vertBase + fBase + f + 2;
				}
				else
				{
					face.mIndices[1] = vertBase + fBase + f + 0;
					face.mIndices[2] = vertBase + fBase + f + 2;
					face.mIndices[0] = vertBase + fBase + f + 1;
				}
				mesh.mFaces.push_back(face);
			}
			fBase += stripLen;
		}
	}
}

std::vector<aiMeshWrapper> AssimpAnimExporter::ExportSkelMesh(CSkelMesh& skelMesh, CSkeleton& skel)
{
	std::vector<aiBoneWrapper> exBones = std::vector<aiBoneWrapper>();
	for (auto& b : skel.bones)
		exBones.push_back(ExportBone(b));

	// 1 mesh per "material"
	std::vector<aiMeshWrapper> meshList = std::vector<aiMeshWrapper>(skelMesh.textures.size() + 1);
	for (int i = 0; i < skelMesh.textures.size() + 1; i++)
	{
		meshList[i].mMaterialIndex = i;
		meshList[i].mBones = exBones;
	}
	int dummyMatIdx = skelMesh.textures.size();
	
	int fptr = 0;
	for (CSkelMeshSection& section : skelMesh.sections)
	{
		float* vertData = &(skelMesh.vertData[fptr]);
		AddSection(meshList[section.textureIndex == 255 ? dummyMatIdx : section.textureIndex], section, vertData);
		fptr += section.vertCount * (section.stride/sizeof(float));
	}

	return meshList;
}

void MergeMesh(aiMeshWrapper& meshA, aiMeshWrapper& meshB)
{
	int vertBase = meshA.mVertices.size();
	meshA.mVertices.insert(meshA.mVertices.end(), meshB.mVertices.begin(), meshB.mVertices.end());
	meshA.mColors.insert(meshA.mColors.end(), meshB.mColors.begin(), meshB.mColors.end());
	meshA.mTextureCoords.insert(meshA.mTextureCoords.end(), meshB.mTextureCoords.begin(), meshB.mTextureCoords.end());
	for (auto& face : meshB.mFaces)
	{
		aiFace newFace = aiFace(face);
		for (int i = 0; i < newFace.mNumIndices; i++) newFace.mIndices[i] += vertBase;
		meshA.mFaces.push_back(newFace);
	}
	for (int i = 0; i < meshB.mBones.size(); i++)
	{
		aiBoneWrapper& BoneA = meshA.mBones.at(i);
		aiBoneWrapper& BoneB = meshB.mBones.at(i);
		for (auto& weight : BoneB.mWeights)
		{
			BoneA.mWeights.emplace_back(vertBase + weight.mVertexId, weight.mWeight);
		}
	}
}

void AssimpAnimExporter::ExportSkelScene(BBS::CSkelModelObject* model, BBS::CBBSAnimSet* anim, std::string modelName, std::string exportPath, ExportFormat format)
{
	std::filesystem::path exportFolder = std::filesystem::path(exportPath);
	exportFolder.append(modelName);
	exportFolder.replace_extension("");

	if (std::filesystem::exists(exportFolder))
	{
		if (!std::filesystem::is_directory(exportFolder))
		{
			// TODO: Error
			std::cerr << "[Export] Export path " << exportPath << " is not a directory!" << std::endl;
			return;
		}
	}
	else
	{
		if (!std::filesystem::create_directories(exportFolder))
		{
			std::cerr << "[Export] Could not create directory " << exportPath << "!" << std::endl;
			return;
		}
	}

	aiSceneWrapper scene = aiSceneWrapper();
	aiNode* root = new aiNode();
	root->mTransformation = aiMatrix4x4();
	scene.mAllNodes.push_back(root);
	scene.mRootNode = root;
	scene.mRootNode->mName.Set(modelName);

	aiNode* tree = Export(*model->skel);
	scene.AddNodeToRoot(tree);

	std::vector<aiMeshWrapper> mesh0 = ExportSkelMesh(*model->mesh0, *model->skel);
	if (model->mesh1 != nullptr)
	{
		std::vector<aiMeshWrapper> mesh1 = ExportSkelMesh(*model->mesh1, *model->skel);
		for (int i = 0; i < mesh1.size(); i++)
		{
			MergeMesh(mesh0.at(i), mesh1.at(i));
		}
	}

	scene.mRootNode->mNumMeshes = mesh0.size();
	scene.mRootNode->mMeshes = new unsigned int[scene.mRootNode->mNumMeshes];
	for (int i = 0; i < scene.mRootNode->mNumMeshes; i++)
	{
		aiMesh* theMesh = mesh0[i].Finish();
		theMesh->mName.Set(modelName);
		scene.mMeshes.push_back(theMesh);
		scene.mRootNode->mMeshes[i] = i;
	}

	for (auto texInfo : model->textureObjects)
	{
		std::filesystem::path outPath = exportFolder;
		outPath.append(texInfo->name.c_str());
		outPath.replace_extension(".png");
		aiString filename = aiString(outPath.filename().string());
		aiString aiName = aiString(texInfo->name);

		// Step 1: Build material
		aiMaterial* mat = new aiMaterial();
		mat->AddProperty(&aiName, AI_MATKEY_NAME);
		mat->AddProperty(&filename, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0));
		scene.mMaterials.push_back(mat);

		// Step 2: Write out texture
		CTexture* tex = texInfo->srcTexture->texture;
		if (stbi_write_png(outPath.string().c_str(), tex->getWidth(), tex->getHeight(), 4, tex->getPixels(), tex->getWidth() * 4) == 0)
		{
			// TODO: error
			std::cerr << "[Export] Failed to write texture " << texInfo->name << std::endl;
		}
	}

	aiMaterial* mat = new aiMaterial();
	aiString n = aiString("dummyMaterial");
	aiColor4D c = aiColor4D(1, 1, 1, 1);
	mat->AddProperty(&n, AI_MATKEY_NAME);
	mat->AddProperty(&c, 1, AI_MATKEY_COLOR_DIFFUSE);
	scene.mMaterials.push_back(mat);

	if (anim != nullptr)
	{
		/*for (auto& animInfo : anim->animInfos)
		{
			if (animInfo.idx != -1)
			{
				scene.mAnimations.push_back(Export(anim->anims[animInfo.idx], animInfo.name, *model->skel));
			}
		}*/
		
		auto& animInfo = anim->animInfos[anim->selectedIdx];
		if (animInfo.idx != -1)
			scene.mAnimations.push_back(Export(anim->anims[animInfo.idx], animInfo.name, *model->skel));
	}
	
	aiScene* finalScene = scene.Finish();

	//finalScene->

	// Step 4: Export scene
	Assimp::Exporter exp = Assimp::Exporter();

	std::filesystem::path outFile = exportFolder;
	outFile.append(modelName);
	outFile.replace_extension(format.ext);

	finalScene->mMetaData = new aiMetadata();
	finalScene->mMetaData->Add("UnitScaleFactor", 100.0);
	//finalScene->mMetaData->Add("CustomFrameRate", 30.0);

	if (AI_SUCCESS != exp.Export(finalScene, format.id, outFile.string(), aiPostProcessSteps::aiProcess_FlipUVs | aiPostProcessSteps::aiProcess_FindInvalidData | aiPostProcessSteps::aiProcess_CalcTangentSpace | aiPostProcessSteps::aiProcess_ValidateDataStructure))
	{
		std::cerr << "[Export] Failed to export fbx file!" << std::endl;
		std::cerr << "[Export] " << exp.GetErrorString() << std::endl;
	}

	delete finalScene;
}