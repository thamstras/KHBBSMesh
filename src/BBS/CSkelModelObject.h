#pragma once
#include "Common.h"
#include "CModelObject.h"
#include "FileTypes/BbsPmo.h"
#include "Core/CoreRender.h"
#include <array>
#include "Core/CSkeleton.h"

namespace BBS
{

	class CSkelModelSection;

	class CSkelModelObject
	{
	public:
		void LoadPmo(PmoFile& file, bool ownsTextures);

		float scale;
		glm::vec4 bbox[8];
		AABBox boundingBox;

		std::vector<CSkelModelSection*> sections;
		std::vector<CSkelModelSection*> transSections;
		std::vector<std::string> textureNames;
		bool ownsTextures;
		std::vector<CTextureInfo*> textureObjects;

		// TODO: CSkelMesh* mesh0;
		// TODO: CSkelMesh* mesh1;
		CSkeleton* skel;

	private:
		// TODO: CSkelMesh* BuildMesh(std::vector<CModelSection*>& sections);
	};

	class CSkelModelSection
	{
	public:
		int textureIndex;
		int vertexCount;
		VertexFlags flags;
		GLenum primativeType;
		std::vector<uint16_t> primCount;
		uint16_t attributes;
		std::array<uint8_t, 8> boneIdxs;
		uint32_t globalColor;

		std::vector<float> vertexData;

		CSkelModelSection();
		void LoadSection(PmoMesh& mesh);
	};

}

