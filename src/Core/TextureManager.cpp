#include "TextureManager.h"

CTextureManager::CTextureManager() : m_textureMap()
{

}

CTextureManager::~CTextureManager()
{
	// TODO
}

bool CTextureManager::AddTexture(std::string name, CTexture* texture)
{
	m_textureMap.emplace(name, std::shared_ptr<CTexture>(texture));
	return true;
}

bool CTextureManager::IsTextureAvailible(std::string& name)
{
	if (m_textureMap.find(name) == m_textureMap.end()) return false;
	return true;
}

void CTextureManager::PruneTextures()
{
	std::vector<std::string> togo = std::vector<std::string>();
	
	for (auto& entry : m_textureMap)
	{
		if (entry.second.use_count() == 1)
			togo.push_back(entry.first);
	}
	
	for (auto& name : togo)
		m_textureMap.erase(name);
}

std::shared_ptr<CTexture> CTextureManager::GetTexture(const std::string& name)
{
	auto itr = m_textureMap.find(name);
	if (itr == m_textureMap.end())
	{
		std::cerr << "[Texture Library] Failed to retrieve shader " << name << std::endl;
		return std::shared_ptr<CTexture>();
	}
	return (itr->second);
}