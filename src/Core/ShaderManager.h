#pragma once
#include "Common.h"
#include "CShader.h"
#include <unordered_map>

struct ShaderDef
{
	std::string shaderName;
	std::string vertexShaderPath;
	std::string fragmentShaderPath;
};

class CShaderManager
{
public:
	CShaderManager();
	CShaderManager(std::vector<ShaderDef>& shaders);
	~CShaderManager();

	bool AddShader(ShaderDef& shaderDef);
	bool IsShaderAvailible(std::string& name);
	std::shared_ptr<CShader> GetShader(const std::string& name);
private:
	std::unordered_map<std::string, std::shared_ptr<CShader>> m_shaderMap;
};