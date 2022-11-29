#pragma once
#include "GLFW/glfw3.h"

namespace BBSMesh
{
	struct MouseData
	{
		double xPos = 0.0;
		double yPos = 0.0;
		float deltaX = 0.0f;
		float deltaY = 0.0f;
		float deltaScroll = 0.0f;
		bool firstData = true;
		bool rawMotionAvailible = false;
	};

	struct WindowData
	{
		GLFWwindow* window = nullptr;
		int width = 0;
		int height = 0;
		bool hasCursorLock = false;
	};
}