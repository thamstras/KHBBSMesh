#include "GraphicsContext.h"

GraphicsContext::GraphicsContext(std::vector<ShaderDef> initialShaders)
{
	ShaderManager = std::make_shared<CShaderManager>(initialShaders);
	TextureManager = std::make_shared<CTextureManager>();
	AllContexts = std::vector<std::shared_ptr<RenderContext>>();
}

std::shared_ptr<RenderContext> GraphicsContext::CreateRenderContext()
{
	std::shared_ptr<RenderContext> context = std::make_shared<RenderContext>();
	context->render.shaderLibrary = ShaderManager;
	context->render.textureLibrary = TextureManager;
	AllContexts.push_back(context);
	return context;
}