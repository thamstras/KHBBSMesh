#pragma once
#include "Common.h"
#include "Core/CAnimationDriver.h"
#include "Core/CSkeleton.h"
#include "Core/Transform.h"

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
	virtual bool NeedsScaleHack() override;

	// NOTE: Assumes we're already inside a window
	void GUI_DrawControls();

private:
	CSkeleton skeleton;
	std::vector<Transform> currTransforms;
};