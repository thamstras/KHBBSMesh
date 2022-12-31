#pragma once
#include "Common.h"
#include "CModelObject.h"
#include "FileTypes/BbsPmo.h"
#include "Core/CoreRender.h"
#include <array>
#include "Core/CSkeleton.h"
#include "Core/CSkelMesh.h"

namespace BBS
{

	class CSkelModelSection;

	class CSkelModelObject : public CRenderObject
	{
	public:
		CSkelModelObject();
		virtual ~CSkelModelObject();

		void LoadPmo(PmoFile& file, bool ownsTextures);
		void LinkExtTextures(std::unordered_map<std::string, CTextureInfo*> textureMap);
		void UpdateTextureOffsets();

		void BuildMesh();
		void DoDraw(RenderContext& context, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale);

		virtual void DoDraw(RenderContext& context) override;
		virtual float CalcZ(const RenderContext& context) const override;

		float scale;
		glm::vec4 bbox[8];
		AABBox boundingBox;

		std::vector<CSkelModelSection*> sections;
		std::vector<CSkelModelSection*> transSections;
		std::vector<std::string> textureNames;
		bool ownsTextures;
		std::vector<CTextureInfo*> textureObjects;

		CSkelMesh* mesh0;
		CSkelMesh* mesh1;
		CSkeleton* skel;

	private:
		CSkelMesh* BuildMesh(std::vector<CSkelModelSection*>& sections);
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

