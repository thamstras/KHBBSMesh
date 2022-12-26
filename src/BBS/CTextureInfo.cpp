#include "CTextureInfo.h"

using namespace BBS;

CTextureInfo::CTextureInfo(PmoTexture& texInfo, CTextureObject* texObj)
{
	srcTexture = texObj;
	name = std::string(texInfo.resourceName, 12);
	scrollSpeed_x = texInfo.scrollU;
	scrollSpeed_y = texInfo.scrollV;
	currentScroll = glm::vec2(0.0f);
}

CTextureInfo::~CTextureInfo()
{
	if (srcTexture)
	{
		delete srcTexture;
		srcTexture = nullptr;
	}
}

void CTextureInfo::Update(float deltaTime, double worldTime)
{
	currentScroll.x = worldTime * scrollSpeed_x * 30.f;
	currentScroll.y = worldTime * scrollSpeed_y * 30.f;
}