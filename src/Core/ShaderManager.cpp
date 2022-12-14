#include "ShaderManager.h"

CShaderManager::CShaderManager() : m_shaderMap()
{

}

CShaderManager::CShaderManager(std::vector<ShaderDef>& shaders)
{
	for (auto itr = shaders.begin(); itr < shaders.end(); itr++)
		this->AddShader(*itr);
}

CShaderManager::~CShaderManager()
{
	// TODO
}

bool CShaderManager::AddShader(ShaderDef& shaderDef)
{
	if (IsShaderAvailible(shaderDef.shaderName))
		return true;

	auto ptr = std::make_shared<CShader>(shaderDef.vertexShaderPath.c_str(), shaderDef.fragmentShaderPath.c_str());
	auto res = m_shaderMap.emplace(shaderDef.shaderName, ptr);

	std::cout << "[Shader Library] Added Shader " << shaderDef.shaderName << std::endl;

	return res.second;
}

bool CShaderManager::IsShaderAvailible(std::string& name)
{
	if (m_shaderMap.find(name) == m_shaderMap.end()) return false;
	return true;
}

std::shared_ptr<CShader> CShaderManager::GetShader(const std::string& name)
{
	auto itr = m_shaderMap.find(name);
	if (itr == m_shaderMap.end())
	{
		std::cerr << "[Shader Library] Failed to retrieve shader " << name << std::endl;
		return std::shared_ptr<CShader>();
	}
	return (itr->second);
}