#pragma once

#include "..\Common.h"
#include "CoreRender.h"

class CDebugObject : public CRenderObject
{
protected:
	float lifetimeRemaining;
public:
	CDebugObject(float life);
	virtual ~CDebugObject();

	float updateLifetime(float deltaTime);
};

class CDebugLineHelper;

class DebugDraw
{
	static std::vector<CDebugObject*> activeDebugObjects;
	static CDebugLineHelper* debugLines;
public:
	static void Update(float deltaTime, double worldTime);
	static void AddDebugObject(CDebugObject* obj);
	static void Teardown();

	static void DebugCube(RenderContext& context, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 color);
	static void DebugLine(RenderContext& context, glm::vec3 start, glm::vec3 end, glm::vec3 color);

	static void DrawLines(RenderContext& context);
};