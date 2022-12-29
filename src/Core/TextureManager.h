#pragma once
#include "Common.h"
#include "CTexture.h"
#include <unordered_map>

class CTextureManager
{
public:
	CTextureManager();
	~CTextureManager();

	bool AddTexture(std::string name, CTexture* texture);
	void PruneTextures();
	bool IsTextureAvailible(std::string& name);
	std::shared_ptr<CTexture> GetTexture(const std::string& name);
private:
	std::unordered_map<std::string, std::shared_ptr<CTexture>> m_textureMap;
};