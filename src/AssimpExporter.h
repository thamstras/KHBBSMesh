#pragma once
#include "Common.h"
#include "Core/CSkelMesh.h"
#include "BBS/CAnimSet.h"
#include "BBS/CSkelModelObject.h"
#include "AssimpInterface.h"

class AssimpAnimExporter
{
public:
	static aiAnimation* Export(BBS::CBBSAnim& anim, std::string name, CSkeleton& skel);
	static aiNodeAnimWrapper Export(BBS::CBoneAnim& boneAnim, std::string boneName, BBS::CBBSAnim& parentAnim);

	static std::vector<aiVectorKey> ExportVectorChannel(BBS::IAnimChannel* x, BBS::IAnimChannel* y, BBS::IAnimChannel* z, int frameRate, int frameCount);
	static std::vector<aiQuatKey> ExportQuatChannel(BBS::IAnimChannel* x, BBS::IAnimChannel* y, BBS::IAnimChannel* z, int frameRate, int frameCount);

	static aiNode* Export(CSkeleton& skeleton);
	static aiNode* ExportSubTree(CBone& bone, CSkeleton& skeleton, aiNode* parent);

	static aiBoneWrapper ExportBone(CBone& bone);
	//static aiBone** ExportSkeleton(CSkeleton& skeleton);

	static std::vector<aiMeshWrapper> ExportSkelMesh(CSkelMesh& skelMesh, CSkeleton& skel);

	static void ExportSkelScene(BBS::CSkelModelObject* model, BBS::CBBSAnimSet* anim, std::string modelName, std::string exportPath, ExportFormat format);
};