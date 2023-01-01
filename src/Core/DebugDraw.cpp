#include "DebugDraw.h"
#include "ShaderManager.h"
#include "CCamera.h"

using namespace std::string_literals;

CDebugObject::CDebugObject(float life) : lifetimeRemaining(life)
{

}

CDebugObject::~CDebugObject()
{

}

float CDebugObject::updateLifetime(float deltaTime)
{
	lifetimeRemaining -= deltaTime;
	return lifetimeRemaining;
}

class CDebugCube : public CDebugObject
{
protected:
	static GLuint VBO;
	static GLuint VAO;
	friend class DebugDraw;	// for Teardown()
public:
	CDebugCube(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, glm::vec3 col, float life);
	virtual ~CDebugCube();

	glm::vec3 pos, rot, scale, col;

	virtual void DoDraw(RenderContext& context) override;
	virtual float CalcZ(const RenderContext& context) const override;
};

class CDebugLineHelper
{
private:
	struct LineVert
	{
		float x, y, z, r, g, b;
	};
	std::vector<LineVert> verts;
	GLuint VBO = 0, VAO = 0;
public:
	CDebugLineHelper();
	~CDebugLineHelper();
	void DoDraw(RenderContext& context);
	void Add(glm::vec3 start, glm::vec3 end, glm::vec3 color);
};

std::vector<CDebugObject*> DebugDraw::activeDebugObjects;
CDebugLineHelper* DebugDraw::debugLines = nullptr;
GLuint CDebugCube::VBO = 0;
GLuint CDebugCube::VAO = 0;

void DebugDraw::DebugCube(RenderContext& context, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 color)
{
	CDebugCube* cube = new CDebugCube(position, rotation, scale, color, 0.0f);
	context.render.overlayDrawList.push_back(cube);
	DebugDraw::AddDebugObject(cube);
}

void DebugDraw::DebugLine(RenderContext& context, glm::vec3 start, glm::vec3 end, glm::vec3 color)
{
	if (debugLines == nullptr) debugLines = new CDebugLineHelper();
	debugLines->Add(start, end, color);
}

float debug_cube_verts[] =
{
	1.0f, 1.0f, 1.0f,	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,	-1.0f, 1.0f, 1.0f,

	-1.0f, -1.0f, -1.0f,	-1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,	1.0f, -1.0f, -1.0f,

	-1.0f, 1.0f, 1.0f,	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, 1.0f,	-1.0f, -1.0f, 1.0f,

	1.0f, -1.0f, 1.0f,	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,	-1.0f, -1.0f, 1.0f,

	1.0f, 1.0f, -1.0f,	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,	-1.0f, 1.0f, -1.0f,
};

CDebugCube::CDebugCube(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, glm::vec3 col, float life)
	: CDebugObject(life), rot(rot), scale(scale), col(col), pos(pos)
{

}

CDebugCube::~CDebugCube()
{

}

void CDebugCube::DoDraw(RenderContext& context)
{
	if (VAO == 0)
	{
		// Create buffers from static data
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), debug_cube_verts, GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		
		glBindVertexArray(0);
	}

	// Draw cube at stored transform using static vao.
	// Suggest using GL_LINES so cube is always wireframe

	// TODO: Debug objects don't move, so all this can be worked out once and then stored.
	glm::quat rot = glm::quat(this->rot);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, this->pos);
	model = glm::rotate(model, glm::angle(rot), glm::axis(rot));
	model = glm::scale(model, this->scale / 2.0f);	// Cube verts define a 'radius' 2 cube, so 1/2 the scale.
	
	std::shared_ptr<CShader> shader = context.render.shaderLibrary->GetShader("constant"s);
	shader->use();
	shader->setMat4("model"s, model);
	shader->setMat4("view"s, context.render.viewMatrix);
	shader->setMat4("projection"s, context.render.projectionMatrix);
	shader->setVec3("color"s, this->col);
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 24);

	context.stats.draw_calls++;
	context.stats.obj_drawn++;
	context.stats.tris_drawn += 24;	// 12 lines, 2 tris per line
}

float CDebugCube::CalcZ(const RenderContext& context) const
{
	return glm::dot(this->pos - context.render.current_camera->Position, context.render.current_camera->Front);
}

CDebugLineHelper::CDebugLineHelper()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVert), (void*)(offsetof(LineVert, x)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVert), (void*)(offsetof(LineVert, r)));

	glBindVertexArray(0);

}

CDebugLineHelper::~CDebugLineHelper()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void CDebugLineHelper::Add(glm::vec3 start, glm::vec3 end, glm::vec3 color)
{
	LineVert a = LineVert();
	a.x = start.x;
	a.y = start.y;
	a.z = start.z;
	a.r = color.r;
	a.g = color.g;
	a.b = color.b;
	verts.push_back(a);

	LineVert b = LineVert();
	b.x = end.x;
	b.y = end.y;
	b.z = end.z;
	b.r = color.r;
	b.g = color.g;
	b.b = color.b;
	verts.push_back(b);
}

void CDebugLineHelper::DoDraw(RenderContext& context)
{
	glBindBuffer(GL_COPY_WRITE_BUFFER, VBO);
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(LineVert) * verts.size(), verts.data(), GL_STREAM_DRAW);

	std::shared_ptr<CShader> shader = context.render.shaderLibrary->GetShader("unlit_vcol"s);
	shader->use();
	shader->setMat4("model"s, glm::mat4(1.0f));
	shader->setMat4("view"s, context.render.viewMatrix);
	shader->setMat4("projection"s, context.render.projectionMatrix);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, verts.size());

	verts.clear();
}

void DebugDraw::AddDebugObject(CDebugObject* obj)
{
	// try find an empty slot first
	for (auto& itr : activeDebugObjects)
	{
		if (itr == nullptr)
		{
			itr = obj;
			return;
		}
	}
	// didn't find an empty slot, add one
	activeDebugObjects.push_back(obj);
}

void DebugDraw::Update(float deltaTime, double worldTime)
{
	// TODO: WE DON'T RESUBMIT DEBUG OBJECTS TO THE DRAW LIST!
	// This isn't a problem right now because no object lives more than 1 frame but still.
	for (int i = 0; i < (int)activeDebugObjects.size(); i++)
	{
		CDebugObject* obj = activeDebugObjects[i];
		if (obj->updateLifetime(deltaTime) < 0.0f)
		{
			delete obj;
			activeDebugObjects[i] = nullptr;
		}
	}
}

/*
	Reaches into all the debug object classes (which DebugDraw is friends with)
	and deletes all the buffers.
*/
void DebugDraw::Teardown()
{
	for (auto& obj : activeDebugObjects)
		delete obj;
	activeDebugObjects.clear();

	glDeleteVertexArrays(1, &CDebugCube::VAO);
	glDeleteBuffers(1, &CDebugCube::VBO);
	CDebugCube::VAO = 0;
	CDebugCube::VBO = 0;

	if (debugLines != nullptr) delete debugLines;
	debugLines = nullptr;
}

void DebugDraw::DrawLines(RenderContext& context)
{
	if (debugLines != nullptr)
		debugLines->DoDraw(context);
}