#include "TextureManager.h"

CTextureManager::CTextureManager() : m_textureMap()
{

}

CTextureManager::~CTextureManager()
{
	// TODO
}

bool CTextureManager::AddTexture(/* TODO */)
{
	
}

bool CTextureManager::IsTextureAvailible(std::string& name)
{
	if (m_textureMap.find(name) == m_textureMap.end()) return false;
	return true;
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