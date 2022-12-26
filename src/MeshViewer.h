#include "Common.h"
#include "Settings.h"
#include "WindStructs.h"
#include "Core/CCamera.h"
#include "GraphicsContext.h"
#include "Core/CoreRender.h"
#include "CFileManager.h"
#include "BBS/CModelObject.h"

namespace BBSMesh
{
	enum class RunState
	{
		NoState,
		InitState,
		RunState,
		ShutdownState,
		QuitState
	};

	class MeshViewer
	{
		typedef void (MeshViewer::*DelayedFunc)();

	public:
		MeshViewer(int argc = 0, char** argv = nullptr);
		~MeshViewer();

		void Run();

		static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
		static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

		void OnFramebufferSizeChanged(GLFWwindow* window, int width, int height);
		void OnMouseMoved(GLFWwindow* window, double xpos, double ypos);
		void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);

		void ProcessInput(GLFWwindow* window, float deltaTime, double worldTime);
	private:
		bool m_shouldQuit = false;
		RunState m_runState = RunState::NoState;
		Settings m_settings;
		WindowData m_window;
		MouseData m_mouse;

		std::unique_ptr<CFileManager> m_fileManager;

		std::shared_ptr<CCamera> m_currentCamera;
		std::unique_ptr<GraphicsContext> m_graphicsContext;
		std::shared_ptr<RenderContext> m_rootRenderContext;

		std::vector<DelayedFunc> m_scheduledDelays;

		std::string m_modalMessage;

		BBS::CModelObject* m_model;

		bool Init();

		void ProcessGUI();
		void Update(float deltaTime, double worldTime);
		void Render();
		void ProcessDelayed();

		void ScheduleDelayedProcess(DelayedFunc func);

		void OpenFile();
		void CloseFile();
		void ExportFile();
		void HideMessageModal();

		void GUI_MenuBar();
		void GUI_SideBar();
		void GUI_Modals();
	};
}