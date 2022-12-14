#pragma once
#include "Common.h"
#include "FileTypes/BbsPmo.h"
#include "Core/CoreRender.h"
#include "Core/CMesh.h"
#include <unordered_map>
#include "CTextureInfo.h"
#include "Utils\Math.h"

namespace BBS
{

	class CModelSection;

	class CModelObject : public CRenderObject
	{
	public:
		CModelObject();
		virtual ~CModelObject();
		void LoadPmo(PmoFile& pmo, bool loadTextures);
		void LoadTexture(PmoTexture& texInfo);
		void LinkExtTextures(std::unordered_map<std::string, CTextureInfo*> textureMap);
		void UpdateTextureOffsets();

		void BuildMesh();
		void DoDraw(RenderContext& context, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale);

		virtual void DoDraw(RenderContext& context) override;
		virtual float CalcZ(const RenderContext& context) const override;

		float scale;
		glm::vec4 bbox[8];
		AABBox boundingBox;

		std::vector<CModelSection*> sections;
		std::vector<CModelSection*> transSections;
		std::vector<std::string> textureNames;
		bool ownsTextures;
		std::vector<CTextureInfo*> textureObjects;

		CMesh* mesh0;
		CMesh* mesh1;

	private:
		CMesh* BuildMesh(std::vector<CModelSection*>& sections);
	};

	struct VertexFlags
	{
		uint8_t hasWeights : 1;
		uint8_t hasTex : 1;
		uint8_t hasVColor : 1;
		uint8_t hasPosition : 1;
		uint8_t hasCColor : 1;
	};

	inline bool operator==(const VertexFlags& left, const VertexFlags& right)
	{
		return (left.hasWeights == right.hasWeights)
			&& (left.hasTex == right.hasTex)
			&& (left.hasVColor == right.hasVColor)
			&& (left.hasPosition == right.hasPosition)
			&& (left.hasCColor == right.hasCColor);
	}

	inline bool operator!=(const VertexFlags& left, const VertexFlags& right)
	{
		return !(left == right);
	}

	VertexFlags Merge(const VertexFlags& a, const VertexFlags& b);

	class CModelSection
	{
	public:
		int textureIndex;
		int vertexCount;
		GLenum primativeType;
		std::vector<uint16_t> primCount;
		VertexFlags flags;
		std::vector<float> vertexData;
		uint16_t attributes;
		uint32_t globalColor;

		CModelSection();
		void LoadSection(PmoMesh& mesh);
	};
}