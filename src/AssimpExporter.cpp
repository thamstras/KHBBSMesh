#include "AssimpExporter.h"
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Utils/Glm2Assimp.h"
#include <filesystem>
#include "Utils/stb_image_write.h"
#include <algorithm>
#include "Utils/BitCast.h"
#include <set>

aiAnimation* AssimpAnimExporter::Export(BBS::Anim const& anim, std::string name, CSkeleton& skel)
{
	aiAnimationWrapper outAnim = aiAnimationWrapper();
	outAnim.mName = name;
	outAnim.mDuration = anim.FrameCount() * anim.FrameRate();
	outAnim.mTicksPerSecond = anim.FrameRate();
	for (int i = 0; i < anim.BoneCount(); i++)
	{
		outAnim.mChannels.push_back(Export(anim.GetRaw(i), skel.bones[i].name, anim));
	}
	return outAnim.Finish();
}

aiNodeAnimWrapper AssimpAnimExporter::Export(BBS::BoneChannel const* boneAnim, std::string boneName, BBS::Anim const& parentAnim)
{
	// info
	aiNodeAnimWrapper outAnim = aiNodeAnimWrapper();
	outAnim.mNodeName = boneName;
	outAnim.mPreState = aiAnimBehaviour_CONSTANT;
	outAnim.mPostState = aiAnimBehaviour_CONSTANT;

	// Position
	auto vc = boneAnim->GetRawTranslate();
	for (int i = 0; i < vc->KeyframeCount(); i++)
	{
		auto kf = vc->GetKeyframe(i);
		outAnim.mPositionKeys.push_back(aiVectorKey(kf.time, Glm2Assimp::Vector3(kf.value)));
	}

	// Rotation
	auto qc = boneAnim->GetRawRotate();
	for (int i = 0; i < qc->KeyframeCount(); i++)
	{
		auto kf = qc->GetKeyframe(i);
		outAnim.mRotationKeys.push_back(aiQuatKey(kf.time, Glm2Assimp::Quat(kf.value)));
	}

	// TODO: Scale Hack
	// Scale
	vc = boneAnim->GetRawScale();
	for (int i = 0; i < vc->KeyframeCount(); i++)
	{
		auto kf = vc->GetKeyframe(i);
		outAnim.mScalingKeys.push_back(aiVectorKey(kf.time, Glm2Assimp::Vector3(kf.value)));
	}

	return outAnim;
}

aiNode* AssimpAnimExporter::Export(CSkeleton& skeleton)
{
	// UNROOT HACK
	/*aiNode* node = new aiNode();
	node->mName.Set("unroot");
	aiNode* rest = ExportSubTree(skeleton.bones[0], skeleton, node);
	node->addChildren(1, &rest);
	return node;*/
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
void AddSection(aiMeshWrapper& mesh, BBS::CSkelModelSection& section, BBS::CSkelModelObject& obj)
{
	int fptr = 0;
	int faceCount;
	
	int sectionBaseIdx = mesh.mVertices.size();
	for (int vi = 0; vi < section.vertexData.size(); vi++)
	{
		BBS::SkelVert& vert = section.vertexData[vi];
		mesh.mVertices.push_back(Glm2Assimp::Vector3(vert.pos * obj.scale));
		mesh.mColors.push_back(Glm2Assimp::Color(vert.color * glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)));
		mesh.mTextureCoords.push_back(Glm2Assimp::Vector3(glm::vec3(vert.tex, 0.0f)));
		for (int i = 0; i < 8; i++)
		{
			if (section.boneIdxs[i] >= 255) continue;
			if (vert.weights[i] <= 0.0f) continue;
			// TODO: Unroot hack
			//mesh.mBones[section.boneIdxs[i] + 1].mWeights.emplace_back(sectionBaseIdx + vi, vert.weights[i]);
			mesh.mBones[section.boneIdxs[i]].mWeights.emplace_back(sectionBaseIdx + vi, vert.weights[i]);
		}
	}

	if (section.primativeType == GL_TRIANGLES)
	{
		assert(section.vertexData.size() % 3 == 0);
		faceCount = section.vertexData.size() / 3;
		for (int f = 0; f < faceCount; f++)
		{
			aiFace face;
			face.mNumIndices = 3;
			face.mIndices = new unsigned int[3];
			face.mIndices[0] = sectionBaseIdx + (f * 3) + 0;
			face.mIndices[1] = sectionBaseIdx + (f * 3) + 1;
			face.mIndices[2] = sectionBaseIdx + (f * 3) + 2;
			mesh.mFaces.push_back(face);
		}
	}
	else
	{
		assert(section.primativeType == GL_TRIANGLE_STRIP);
		int fBase = 0;
		for (unsigned int stripLen : section.primCounts)
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
					face.mIndices[0] = sectionBaseIdx + fBase + f + 0;
					face.mIndices[1] = sectionBaseIdx + fBase + f + 1;
					face.mIndices[2] = sectionBaseIdx + fBase + f + 2;
				}
				else
				{
					face.mIndices[0] = sectionBaseIdx + fBase + f + 0;
					face.mIndices[1] = sectionBaseIdx + fBase + f + 2;
					face.mIndices[2] = sectionBaseIdx + fBase + f + 1;
				}
				mesh.mFaces.push_back(face);
			}
			fBase += stripLen;
		}
	}
}

std::vector<aiMeshWrapper> AssimpAnimExporter::ExportSkelMesh(BBS::CSkelModelObject& skelMesh)
{
	std::vector<aiBoneWrapper> exBones = std::vector<aiBoneWrapper>();
	if (skelMesh.skel != nullptr)
	{
		// UNROOT HACK
		//exBones.push_back(aiBoneWrapper());
		//exBones[0].mName = "unroot";
		for (auto& b : skelMesh.skel->bones)
			exBones.push_back(ExportBone(b));
	}

	// 1 mesh per "material"
	std::vector<aiMeshWrapper> meshList = std::vector<aiMeshWrapper>(skelMesh.textureObjects.size() + 1);
	for (int i = 0; i < skelMesh.textureObjects.size() + 1; i++)
	{
		meshList[i].mMaterialIndex = i;
		meshList[i].mBones = exBones;
	}
	int dummyMatIdx = skelMesh.textureObjects.size();
	
	for (BBS::CSkelModelSection* section : skelMesh.sections)
		AddSection(meshList[section->textureIndex == 255 ? dummyMatIdx : section->textureIndex], *section, skelMesh);

	if (skelMesh.transSections.size() > 0)
	{
		for (BBS::CSkelModelSection* section : skelMesh.transSections)
			AddSection(meshList[section->textureIndex == 255 ? dummyMatIdx : section->textureIndex], *section, skelMesh);
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

void AssimpAnimExporter::ExportSkelScene(BBS::CSkelModelObject* model, BBS::CBBSAnimationProvider* anim, std::string modelName, std::string exportPath, ExportFormat format)
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
	root->mName.Set(modelName);

	if (model->skel != nullptr)
	{
		aiNode* tree = Export(*model->skel);
		root->addChildren(1, &tree);
	}

	std::vector<aiMeshWrapper> mesh = ExportSkelMesh(*model);

	root->mNumMeshes = mesh.size();
	root->mMeshes = new unsigned int[root->mNumMeshes];
	for (int i = 0; i < root->mNumMeshes; i++)
	{
		aiMesh* theMesh = mesh[i].Finish();
		theMesh->mName.Set(modelName);
		scene.mMeshes.push_back(theMesh);
		root->mMeshes[i] = i;
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
		int i = aiShadingMode_Unlit;
		mat->AddProperty(&i, 1, AI_MATKEY_SHADING_MODEL);
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

	if (anim != nullptr && model->skel != nullptr)
	{
		/*for (auto& animInfo : anim->animInfos)
		{
			if (animInfo.idx != -1)
			{
				scene.mAnimations.push_back(Export(anim->anims[animInfo.idx], animInfo.name, *model->skel));
			}
		}*/
		
		BBS::Anim const* animToExport = anim->GetCurrAnim();
		if (animToExport != nullptr)
			scene.mAnimations.push_back(Export(*animToExport, anim->GetCurrAnimName(), *model->skel));
	}
	
	aiScene* finalScene = scene.Finish();

	//finalScene->

	// Step 4: Export scene
	Assimp::Exporter exp = Assimp::Exporter();

	std::filesystem::path outFile = exportFolder;
	outFile.append(modelName);
	outFile.replace_extension(format.ext);

	finalScene->mMetaData = new aiMetadata();
	// FBX settings
	finalScene->mMetaData->Add("UnitScaleFactor", 100.0);
	finalScene->mMetaData->Add("TimeMode", 14);	// Custom frame rate (30fps is 6, 24fps is 11)
	finalScene->mMetaData->Add("CustomFrameRate", 30.0);
	finalScene->mMetaData->Add("InheritMode", 2); // Disable scale inheritance

	if (AI_SUCCESS != exp.Export(finalScene, format.id, outFile.string(), aiPostProcessSteps::aiProcess_FlipUVs | aiPostProcessSteps::aiProcess_CalcTangentSpace | aiPostProcessSteps::aiProcess_JoinIdenticalVertices))
	{
		std::cerr << "[Export] Failed to export fbx file!" << std::endl;
		std::cerr << "[Export] " << exp.GetErrorString() << std::endl;
	}

	delete finalScene;
}