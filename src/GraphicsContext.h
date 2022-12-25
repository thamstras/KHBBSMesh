#pragma once
#include "Core/CoreRender.h"
#include "Core/ShaderManager.h"
#include "Core/TextureManager.h"

// TODO: Move to core
class GraphicsContext
{
public:
	GraphicsContext(std::vector<ShaderDef> initialShaders);

	std::unique_ptr<CShaderManager> ShaderManager;

	std::unique_ptr<CTextureManager> TextureManager;

	std::shared_ptr<RenderContext> CreateRenderContext();

private:
	std::vector<std::shared_ptr<RenderContext>> AllContexts;
};