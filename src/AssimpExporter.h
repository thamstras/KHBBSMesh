#pragma once
#include "Common.h"
#include "Core/CSkelMesh.h"
#include "BBS/CAnimSet.h"
#include "BBS/CSkelModelObject.h"
#include "AssimpInterface.h"

class AssimpAnimExporter
{
public:
	static aiAnimation* Export(BBS::Anim const& anim, std::string name, CSkeleton& skel);
	static aiNodeAnimWrapper Export(BBS::BoneChannel const* boneAnim, std::string boneName, BBS::Anim const& parentAnim);

	static aiNode* Export(CSkeleton& skeleton);
	static aiNode* ExportSubTree(CBone& bone, CSkeleton& skeleton, aiNode* parent);

	static aiBoneWrapper ExportBone(CBone& bone);
	//static aiBone** ExportSkeleton(CSkeleton& skeleton);

	static std::vector<aiMeshWrapper> ExportSkelMesh(BBS::CSkelModelObject& skelMesh);

	static void ExportSkelScene(BBS::CSkelModelObject* model, BBS::CBBSAnimationProvider* anim, std::string modelName, std::string exportPath, ExportFormat format);
};