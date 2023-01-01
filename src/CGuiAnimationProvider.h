#pragma once
#include "Common.h"
#include "Core/CAnimationDriver.h"
#include "Core/CSkeleton.h"

struct Transform
{
public:
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	Transform(glm::vec3 p, glm::vec3 r, glm::vec3 s);
	Transform();
	glm::mat4 GetTransform();
	void SetTransform(glm::mat4 transform);
};

class CGUIAnimationProvider : public CAnimationProvider
{
public:
	CGUIAnimationProvider(CSkeleton skel);
	virtual ~CGUIAnimationProvider();

	virtual int BoneCount() override;
	virtual void Update(float deltaTime, double worldTime) override;
	virtual glm::mat4 GetBone(int idx) override;

	virtual void SetAnimTime(float time) override;
	virtual void SetPlayRate(float rate) override;
	virtual void SetPlaying(bool isPlaying) override;

	// NOTE: Assumes we're already inside a window
	void GUI_DrawControls();

private:
	CSkeleton skeleton;
	std::vector<Transform> currTransforms;
};