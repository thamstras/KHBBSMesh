#include "CoreRender.h"
#include "CCamera.h"
#include <algorithm>

CRenderObject::~CRenderObject() {};

void RenderContext::AddToDrawList(ERenderLayer pass, CRenderObject* obj)
{
	switch (pass)
	{
	case LAYER_SKY:
		this->render.skyDrawList.push_back(obj);
		return;
	case LAYER_STATIC:
		this->render.staticDrawList.push_back(obj);
		return;
	case LAYER_DYNAMIC:
		this->render.dynamicDrawList.push_back(obj);
		return;
	case LAYER_OVERLAY:
		this->render.overlayDrawList.push_back(obj);
		return;
	case LAYER_GUI:
		this->render.guiDrawList.push_back(obj);
		return;
	default:
		throw std::exception("FATAL: Invalid pass in AddToDrawList");
	}
}

void RenderContext::NewFrame()
{
	stats.draw_calls = 0;
	stats.obj_drawn = 0;
	stats.tris_drawn = 0;

	render.skyDrawList.clear();
	render.staticDrawList.clear();
	render.dynamicDrawList.clear();
	render.overlayDrawList.clear();
	render.guiDrawList.clear();

	render.viewMatrix = render.current_camera->GetViewMatrix();
	render.projectionMatrix = glm::perspective(glm::radians(render.current_camera->Zoom), (float)env.frameWidth / (float)env.frameHeight, render.nearClip, render.farClip);
	render.skyViewMatrix = glm::mat4(glm::mat3(render.viewMatrix));
}

void RenderContext::Render()
{
	if (render.no_cull)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

	if (render.wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(env.clearColor.r, env.clearColor.g, env.clearColor.b, env.clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::shared_ptr<CShader> highlightShader = render.shaderLibrary->GetShader(render.highlight_shader);
	highlightShader->use();
	highlightShader->setVec4("color", debug.highlight_color);

	std::shared_ptr<CShader> standardShader = render.shaderLibrary->GetShader(render.default_shader);
	standardShader->use();
	standardShader->setVec4("fog_color", env.fogColor);
	if (render.no_fog)
	{
		standardShader->setFloat("fog_near", render.farClip - FLT_EPSILON);
		standardShader->setFloat("fog_far", render.farClip);
	}
	else
	{
		standardShader->setFloat("fog_near", env.fogNear);
		standardShader->setFloat("fog_far", env.fogFar);
	}

	auto comp = [this](CRenderObject* const& a, CRenderObject* const& b) {
		return a->CalcZ(*this) > b->CalcZ(*this);
	};

	// TODO: Cap z depth to far clip. (or maybe 2x far clip)
	// Not sure how much it'll help for most maps (Other than rumble racing) but
	// probably a good idea none the less.

	glDisable(GL_DEPTH_TEST);
	render.currentPass = LAYER_SKY;
	for (CRenderObject* const& renderObject : render.skyDrawList)
		renderObject->DoDraw(*this);
	glEnable(GL_DEPTH_TEST);

	render.currentPass = LAYER_STATIC;
	std::sort(render.staticDrawList.begin(), render.staticDrawList.end(), comp);
	for (CRenderObject* const& renderObject : render.staticDrawList)
		renderObject->DoDraw(*this);

	//glDepthMask(GL_FALSE);
	render.currentPass = LAYER_DYNAMIC;
	std::sort(render.dynamicDrawList.begin(), render.dynamicDrawList.end(), comp);
	for (CRenderObject* const& renderObject : render.dynamicDrawList)
		renderObject->DoDraw(*this);

	glDepthMask(GL_FALSE);
	render.currentPass = LAYER_OVERLAY;
	for (CRenderObject* const& renderObject : render.overlayDrawList)
		renderObject->DoDraw(*this);
	glDepthMask(GL_TRUE);

	glDisable(GL_DEPTH_TEST);
	render.currentPass = LAYER_GUI;
	for (CRenderObject* const& renderObject : render.guiDrawList)
		renderObject->DoDraw(*this);
	glEnable(GL_DEPTH_TEST);
}