#pragma once
#include "Common.h"
#include "CModelObject.h"
#include "FileTypes/BbsPmo.h"
#include "Core/CoreRender.h"
#include <array>
#include "Core/CSkeleton.h"
#include "Core/CSkelMesh.h"
#include "Core/CAnimationDriver.h"

namespace BBS
{

	class CSkelModelSection;
	struct SkelVert;

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

		void SetAnimDriver(CAnimationDriver* driver);

		uint8_t num;
		uint8_t group;
		uint16_t fileTriCount;
		uint16_t fileVertCount;

		float scale;
		glm::vec4 bbox[8];
		AABBox boundingBox;
		int triCount;
		int vertCount;

		std::vector<CSkelModelSection*> sections;
		std::vector<CSkelModelSection*> transSections;
		std::vector<std::string> textureNames;
		bool ownsTextures;
		std::vector<CTextureInfo*> textureObjects;

		CSkelMesh* mesh0;
		CSkelMesh* mesh1;
		CSkeleton* skel;
		CAnimationDriver* animDriver;

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
		std::vector<uint16_t> primCounts;
		uint16_t attributes;
		std::array<uint8_t, 8> boneIdxs;
		uint32_t globalColor;
		uint8_t group;
		std::string formatStr;

		std::vector<SkelVert> vertexData;

		CSkelModelSection();
		void LoadSection(PmoMesh& mesh, bool hasSkeleton);
	};

	struct SkelVert
	{
		glm::vec3 pos;
		glm::vec4 color;
		glm::vec2 tex;
		std::array<float, 8> weights;
	};

}

