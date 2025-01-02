#include "CSkelModelObject.h"
#include "Utils/BitCast.h"
#include <algorithm>
#include <numeric>
#include <format>

using namespace BBS;

CSkelModelObject::CSkelModelObject()
	: boundingBox(glm::vec3(0.0f), glm::vec3(0.0f))
{
	scale = 0.0f;
	for (int i = 0; i < 8; i++) bbox[i] = glm::vec4(0.0f);
	ownsTextures = false;
	mesh0 = nullptr;
	mesh1 = nullptr;
}

CSkelModelObject::~CSkelModelObject()
{
	if (mesh0)
	{
		delete mesh0;
		mesh0 = nullptr;
	}

	if (mesh1)
	{
		delete mesh1;
		mesh1 = nullptr;
	}

	if (ownsTextures)
	{
		for (auto texture : textureObjects)
			delete texture;
	}
}

void CSkelModelObject::LoadPmo(PmoFile& pmo, bool loadTextures)
{
	num = pmo.header.num;
	group = pmo.header.group;
	fileTriCount = pmo.header.triangleCount;
	fileVertCount = pmo.header.vertexCount;
	
	scale = pmo.header.scale;

	// read bounding box
	for (int i = 0; i < 8; i++)
	{
		bbox[i] = glm::vec4(pmo.header.boundingBox[(i * 4) + 0],
			pmo.header.boundingBox[(i * 4) + 1],
			pmo.header.boundingBox[(i * 4) + 2],
			pmo.header.boundingBox[(i * 4) + 3]);
	}

	glm::vec4 min = glm::vec4(FLT_MAX);
	glm::vec4 max = glm::vec4(-FLT_MAX);
	for (auto& vert : bbox)
	{
		min = glm::min(min, vert);
		max = glm::max(max, vert);
	}

	boundingBox = AABBox(glm::vec3(min), glm::vec3(max));

	if (pmo.hasMesh0())
	{
		sections.reserve(pmo.mesh0.size());
		for (PmoMesh& mesh : pmo.mesh0)
		{
			if (mesh.header.vertexCount == 0) continue;
			CSkelModelSection* section = new CSkelModelSection();
			section->LoadSection(mesh);
			sections.push_back(section);
		}
	}

	if (pmo.hasMesh1())
	{
		transSections.reserve(pmo.mesh1.size());
		for (PmoMesh& mesh : pmo.mesh1)
		{
			if (mesh.header.vertexCount == 0) continue;
			CSkelModelSection* section = new CSkelModelSection();
			section->LoadSection(mesh);
			transSections.push_back(section);
		}
	}

	ownsTextures = loadTextures;
	textureNames.reserve(pmo.textures.size());
	for (PmoTexture& tex : pmo.textures)
	{
		this->textureNames.push_back(std::string(tex.resourceName, 12));
		//if (loadTextures) LoadTexture(tex);
	}

	std::vector<CBone> bones = std::vector<CBone>();
	if (pmo.hasSkeleton())
	{
		for (auto& pmoBone : pmo.skeleton.joints)
		{
			CBone bone = CBone();
			bone.index = pmoBone.index;
			bone.parentIndex = pmoBone.parent;
			if (bone.parentIndex == UINT16_MAX) bone.parentIndex = -1;
			bone.name = std::string(pmoBone.name, 0x10);
			bone.transform = Matrix44FromArrays(pmoBone.transform);
			bone.inverseTransform = Matrix44FromArrays(pmoBone.transformInverse);
			bones.push_back(bone);
		}
	}

	skel = new CSkeleton();
	skel->bones = bones;
}

void CSkelModelObject::LinkExtTextures(std::unordered_map<std::string, CTextureInfo*> textureMap)
{
	if (ownsTextures) return;

	textureObjects.reserve(textureNames.size());
	for (std::string& texName : textureNames)
	{
		CTextureInfo* texObj = textureMap.at(texName);
		textureObjects.push_back(texObj);
	}
}

void CSkelModelObject::UpdateTextureOffsets()
{
	// TODO: we could store an appropriatly sized vector on the model and just update it
	// once per frame. Or we could drop the scrolls right into the mesh's vectors and skip
	// the multiple implicit loops in the copy operators.
	std::vector<glm::vec2> texOffs;
	texOffs.reserve(textureObjects.size());
	for (auto texture : textureObjects)
	{
		texOffs.push_back(texture->currentScroll);
	}
	if (mesh0) mesh0->uvOffsets = texOffs;
	if (mesh1) mesh1->uvOffsets = texOffs;
}

CSkelModelSection::CSkelModelSection()
{
	textureIndex = 0;
	vertexCount = 0;
	flags = { 0 };
	primativeType = GL_TRIANGLES;
	attributes = 0;
	globalColor = 0xFF808080;
}

static float ReadUInt8(uint8_t* data, uint8_t& readPtr)
{
	uint8_t val = data[readPtr++];
	return val;
}

static float ReadUNorm8(uint8_t* data, uint8_t& readPtr)
{
	uint8_t val = data[readPtr++];
	return val / (float)(UINT8_MAX - 1);
}

static float ReadNorm8(uint8_t* data, uint8_t& readPtr)
{
	uint8_t uval = data[readPtr++];
	int8_t val = bit_cast<int8_t, uint8_t>(uval);
	return (2 * val + 1) / (float)(UINT8_MAX - 1);
}

static uint16_t ReadUInt16(uint8_t* data, uint8_t& readPtr)
{
	uint16_t val = (data[readPtr++] << 0) | (data[readPtr++] << 8);
	return val;
}

static float ReadUNorm16(uint8_t* data, uint8_t& readPtr)
{
	uint16_t val = (data[readPtr++] << 0) | (data[readPtr++] << 8);
	return val / (float)(UINT16_MAX - 1);
}

static float ReadNorm16(uint8_t* data, uint8_t& readPtr)
{
	uint16_t uval = (data[readPtr++] << 0) | (data[readPtr++] << 8);
	int16_t val = bit_cast<int16_t, uint16_t>(uval);
	return (2 * val + 1) / (float)(UINT16_MAX - 1);
}

static float ReadFloat(uint8_t* data, uint8_t& readPtr)
{
	uint32_t uval = (data[readPtr++] << 0) | (data[readPtr++] << 8)
		| (data[readPtr++] << 16) | (data[readPtr++] << 24);
	return bit_cast<float, uint32_t>(uval);
}

void CSkelModelSection::LoadSection(PmoMesh& mesh)
{
	uint32_t constDiffuse = 0;
	int elementCount = 0;

	PmoVertexFormatFlags format = mesh.header.vertexFormat;

	this->textureIndex = mesh.header.textureIndex;
	this->vertexCount = mesh.header.vertexCount;
	switch (format.primative)
	{
	case 3: this->primativeType = GL_TRIANGLES; break;
	case 4: this->primativeType = GL_TRIANGLE_STRIP; break;
	default:
		break;
	}
	this->attributes = mesh.header.attributes;
	this->group = mesh.header.group;

	if (format.color != 0)
	{
		this->flags.hasVColor = 1;
		elementCount += 4;
	}
	if (format.texCoord != 0)
	{
		this->flags.hasTex = 1;
		elementCount += 2;
	}
	if (format.weights != 0)
	{
		this->flags.hasWeights = 1;
		elementCount += format.skinning + 1;
	}
	this->flags.hasPosition = 1;
	elementCount += 3;

	if (format.weights)
	{
		for (int i = 0; i < 8; i++) this->boneIdxs[i] = mesh.header.jointIndices[i];
	}

	if (format.diffuse != 0)
	{
		this->globalColor = mesh.header.diffuseColor;
		this->flags.hasCColor = 1;
	}

	if (mesh.header.triStripCount > 0)
	{
		this->primCounts = mesh.header.triStripLengths;
	}
	else
	{
		primCounts.push_back(mesh.header.vertexCount);
	}

	if (format.texCoord > 0)
		formatStr.append(std::format("T{}", (int)format.texCoord));
	if (format.color > 0)
		formatStr.append(std::format("C{}", (int)format.color));
	if (format.normal > 0)
		formatStr.append(std::format("N{}", (int)format.normal));
	if (format.position > 0)
		formatStr.append(std::format("P{}", (int)format.position));
	if (format.weights > 0)
		formatStr.append(std::format("W{}", (int)format.weights));
	if (format.index > 0)
		formatStr.append(std::format("I{}", (int)format.index));
	if (formatStr.length() > 0)
		formatStr.append(1, ' ');
	if (format.weights > 0)
		formatStr.append(std::format("S{}", (int)format.skinning + 1));
	if (format.morphing > 0)
		formatStr.append(std::format("M{}", (int)format.morphing));
	if (formatStr.length() > 0)
		formatStr.append(1, ' ');
	if (format.skipTransform)
		formatStr.append("St");
	if (format.diffuse)
		formatStr.append(1, 'D');
	formatStr.append(std::format("Pr{}", (int)format.primative));

	// read verts
	uint8_t* pData = mesh.vertexData.data();
	vertexData.reserve(mesh.header.vertexCount);
	for (int vi = 0; vi < mesh.header.vertexCount; vi++)
	{
		uint8_t* pVertStart = pData + (vi * mesh.header.vertexSize);
		uint8_t readPtr = 0;
		SkelVert vert{};

		switch (format.weights)
		{
		case 0: // none 
			break;
		case 1: // uint8
			for (int i = 0; i < 8; i++)
			{
				if (i <= format.skinning)
					vert.weights[i] = ReadUNorm8(pVertStart, readPtr);
				else
					vert.weights[i] = 0.0f;
			}
			break;
		default:
			throw std::runtime_error("Bad weights format");
		}

		switch (format.texCoord)
		{
		case 0: // none
			break;
		case 1: // uint8
			vert.tex.x = ReadNorm8(pVertStart, readPtr);
			vert.tex.y = ReadNorm8(pVertStart, readPtr);
			break;
		case 2:	// uint16
			readPtr += ((0x2 - (readPtr & 0x1)) & 0x1);
			vert.tex.x = ReadNorm16(pVertStart, readPtr);
			vert.tex.y = ReadNorm16(pVertStart, readPtr);
			break;
		case 3: // float
			readPtr += ((0x4 - (readPtr & 0x3)) & 0x3);
			vert.tex.x = ReadFloat(pVertStart, readPtr);
			vert.tex.y = ReadFloat(pVertStart, readPtr);
			break;
		default:
			throw new std::runtime_error("Bad tex format");
		}

		// TODO: Re-check color calculations
		uint16_t temp16;
		switch (format.color)
		{
		case 0: // none
			break;
		case 4: // BRG-5650
			readPtr += ((0x2 - (readPtr & 0x1)) & 0x1);
			temp16 = ReadUInt16(pVertStart, readPtr);
			vert.color.r = (((float)((temp16 & 0xF800) >> 11) / 31.f) * 255.f);
			vert.color.g = (((float)((temp16 & 0x07E0) >> 5) / 63.f) * 255.f);
			vert.color.b = (((float)((temp16 & 0x001F) >> 0) / 31.f) * 255.f);
			vert.color.a = (0xFF);
			break;
		case 5: // ABGR-5551
			readPtr += ((0x2 - (readPtr & 0x1)) & 0x1);
			temp16 = ReadUInt16(pVertStart, readPtr);
			vert.color.r = (((float)((temp16 & 0xF800) >> 11) / 31.f) * 255.f);
			vert.color.g = (((float)((temp16 & 0x07C0) >> 6) / 31.f) * 255.f);
			vert.color.b = (((float)((temp16 & 0x003E) >> 1) / 31.f) * 255.f);
			vert.color.a = ((temp16 & 0x8000) ? (0xFF) : (0));
			break;
		case 6: // ABGR-4444
			readPtr += ((0x2 - (readPtr & 0x1)) & 0x1);
			temp16 = ReadUInt16(pVertStart, readPtr);
			vert.color.r = (((float)((temp16 & 0xF000) >> 12) / 15.f) * 255.f);
			vert.color.g = (((float)((temp16 & 0x0F00) >> 8) / 15.f) * 255.f);
			vert.color.b = (((float)((temp16 & 0x00F0) >> 4) / 15.f) * 255.f);
			vert.color.a = (((float)((temp16 & 0x000F) >> 0) / 15.f) * 255.f);
			break;
		case 7: // ABGR-8888
			readPtr += ((0x4 - (readPtr & 0x3)) & 0x3);
			vert.color.r = ((float)ReadUInt8(pVertStart, readPtr) / 127.0f);
			vert.color.g = ((float)ReadUInt8(pVertStart, readPtr) / 127.0f);
			vert.color.b = ((float)ReadUInt8(pVertStart, readPtr) / 127.0f);
			vert.color.a = ((float)ReadUInt8(pVertStart, readPtr) / 255.0f);
			break;
		default:
			throw std::runtime_error("Bad color format");
		}

		switch (format.normal)
		{
		case 1: readPtr += 3; break;
		case 2: readPtr += 6; break;
		case 3: readPtr += 12; break;
		}

		switch (format.position)
		{
		case 1:
			vert.pos.x = ReadNorm8(pVertStart, readPtr);
			vert.pos.y = ReadNorm8(pVertStart, readPtr);
			vert.pos.z = ReadNorm8(pVertStart, readPtr);
			break;
		case 2:
			readPtr += ((0x2 - (readPtr & 0x1)) & 0x1);
			vert.pos.x = ReadNorm16(pVertStart, readPtr);
			vert.pos.y = ReadNorm16(pVertStart, readPtr);
			vert.pos.z = ReadNorm16(pVertStart, readPtr);
			break;
		case 3:
			readPtr += ((0x4 - (readPtr & 0x3)) & 0x3);
			vert.pos.x = ReadFloat(pVertStart, readPtr);
			vert.pos.y = ReadFloat(pVertStart, readPtr);
			vert.pos.z = ReadFloat(pVertStart, readPtr);
			break;
		default:
			throw std::runtime_error("Bad position format");
		}

		vertexData.push_back(vert);
	}
}

CSkelMesh* CSkelModelObject::BuildMesh(std::vector<CSkelModelSection*>& sections)
{
	if (sections.size() == 0) return nullptr;

	CSkelMesh* mesh = new CSkelMesh();

	int totalVcount = 0;
	for (CSkelModelSection* sec : sections)
	{
		totalVcount += sec->vertexCount;
	}
	mesh->vertCount = totalVcount;
	mesh->vertData.reserve(26 * totalVcount);

	for (CSkelModelSection* modelSection : sections)
	{
		CSkelMeshSection meshSection;
		meshSection.vertCount = modelSection->vertexCount;
		// TODO: Dummy (idx: -1) texture
		meshSection.textureIndex = modelSection->textureIndex;
		meshSection.stride = (26 * sizeof(float));
		meshSection.twoSided = modelSection->attributes & ATTR_BACK;
		meshSection.blend = modelSection->attributes & ATTR_BLEND_MASK;
		for (uint16_t kick : modelSection->primCounts)
		{
			meshSection.kickList.push_back((unsigned int)kick);
		}
		meshSection.primType = modelSection->primativeType;
		mesh->sections.push_back(meshSection);
		int rp = 0;
		for (SkelVert& vert : modelSection->vertexData)
		{
			if (modelSection->flags.hasWeights)
			{
				for (int b = 0; b < 8; b++)
				{
					// We need to upcast the uint8 to a full int because we can only bit_cast
					// between same sized types
					int idx = modelSection->boneIdxs[b];
					mesh->vertData.push_back(bit_cast<float, int>(idx));
				}
			}
			else
			{
				for (int b = 0; b < 8; b++)
					mesh->vertData.push_back((float)0);
			}
			for (int w = 0; w < 8; w++)
				mesh->vertData.push_back(vert.weights[w]);

			mesh->vertData.push_back(vert.tex.x);
			mesh->vertData.push_back(vert.tex.y);

			// TODO: Recheck color calculations
			if (modelSection->flags.hasVColor)
			{
				mesh->vertData.push_back(vert.color.r);
				mesh->vertData.push_back(vert.color.g);
				mesh->vertData.push_back(vert.color.b);
				mesh->vertData.push_back(vert.color.a);
			}
			else if (modelSection->flags.hasCColor)
			{
				mesh->vertData.push_back((float)((modelSection->globalColor & 0x000000FF) >> 0) / 128.f);
				mesh->vertData.push_back((float)((modelSection->globalColor & 0x0000FF00) >> 8) / 128.f);
				mesh->vertData.push_back((float)((modelSection->globalColor & 0x00FF0000) >> 16) / 128.f);
				mesh->vertData.push_back((float)((modelSection->globalColor & 0xFF000000) >> 24) / 255.f);
			}
			else
			{
				mesh->vertData.push_back((float)0x80 / 128.f);
				mesh->vertData.push_back((float)0x80 / 128.f);
				mesh->vertData.push_back((float)0x80 / 128.f);
				mesh->vertData.push_back((float)0xFF / 255.f);
			}

			mesh->vertData.push_back(vert.pos.x * this->scale);
			mesh->vertData.push_back(vert.pos.y * this->scale);
			mesh->vertData.push_back(vert.pos.z * this->scale);
			mesh->vertData.push_back(1.0f);
		}

		mesh->polyCount += std::accumulate(modelSection->primCounts.begin(), modelSection->primCounts.end(), 0);
	}
	mesh->Build();
	return mesh;
}

void CSkelModelObject::BuildMesh()
{
	mesh0 = BuildMesh(sections);
	mesh1 = BuildMesh(transSections);

	if (mesh0 == nullptr && mesh1 == nullptr) return;

	std::vector<CTexture*> textureList;
	textureList.reserve(textureObjects.size());
	for (auto texEntry : this->textureObjects)
		textureList.push_back(texEntry->srcTexture->texture);
	if (mesh0 != nullptr) mesh0->textures = textureList;
	if (mesh1 != nullptr) mesh1->textures = textureList;
}

void CSkelModelObject::DoDraw(RenderContext& render, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
{
	if (mesh0) mesh0->Draw(render, pos, rot, scale);
	if (mesh1) mesh1->Draw(render, pos, rot, scale);
}

void CSkelModelObject::DoDraw(RenderContext& context)
{
	DoDraw(context, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
}

float CSkelModelObject::CalcZ(const RenderContext& context) const
{
	return context.render.nearClip + FLT_EPSILON;
}

void CSkelModelObject::SetAnimDriver(CAnimationDriver* driver)
{
	animDriver = driver;
	if (mesh0 != nullptr)
		mesh0->driver = driver;
	if (mesh1 != nullptr)
		mesh1->driver = driver;
}