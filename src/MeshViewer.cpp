#include "MeshViewer.h"

#include "GLFW/glfw3.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Core/DebugDraw.h"

using namespace BBSMesh;

constexpr float MAX_DELTA_TIME = 0.2f;

/* 
	### TODO ###
	[X] Finish event handling
	[X] New FileManager
	[X] struct GraphicsContext + CTextureManager
	[X] BBS/CSkelModelObject + Core/CSkelMesh
	[X] Draw mesh
	[X] CAnimationDriver - updates a skeleton with animation data
	[X] Initial GUI - modify joint transforms (manually 'animate')
	[X] Draw skeleton (Will need to sort out DebugDrawLine properly, eg: draw all the lines in one GL_LINES draw call via a GL_STREAM_DRAW buffer)
	[ ] BBS PAM File reader
	[ ] CPAMAnimationProvider - provides animation data to driver
*/

MeshViewer::MeshViewer(int argc, char** argv)
{
	// TODO:
	// 1) Load settings from ini
	// 2) Load settings from command line
	// 3) Verify preconditions?

	m_fileManager = std::make_unique<CFileManager>();
}

MeshViewer::~MeshViewer()
{
	// TODO
	// Shouldn't be called until shutdown anyway...
}

void MeshViewer::Run()
{
	if (m_runState == RunState::QuitState)
		throw std::exception("Invalid run state on start");

	m_shouldQuit = false;
	m_runState = RunState::InitState;

	// TODO: return value
	if (!Init())
	{
		throw std::exception("Init failure");
	}

	std::vector<ShaderDef> shaders =
	{
		{
			"unlit_vcol_tex",
			m_fileManager->GetResourcePath(EResourceType::RSRC_SHADER, "unlit_vcol_tex.vert.glsl"),
			m_fileManager->GetResourcePath(EResourceType::RSRC_SHADER, "unlit_vcol_tex.frag.glsl")
		},
		{
			"unlit_vcol_tex_skel",
			m_fileManager->GetResourcePath(EResourceType::RSRC_SHADER, "unlit_vcol_tex_skel.vert.glsl"),
			m_fileManager->GetResourcePath(EResourceType::RSRC_SHADER, "unlit_vcol_tex.frag.glsl")
		},
		{
			"constant",
			m_fileManager->GetResourcePath(EResourceType::RSRC_SHADER, "constant.vert.glsl"),
			m_fileManager->GetResourcePath(EResourceType::RSRC_SHADER, "constant.frag.glsl")
		},
		{
			"unlit_vcol",
			m_fileManager->GetResourcePath(EResourceType::RSRC_SHADER, "unlit_vcol.vert.glsl"),
			m_fileManager->GetResourcePath(EResourceType::RSRC_SHADER, "unlit_vcol.frag.glsl")
		}
	};

	m_graphicsContext = std::make_unique<GraphicsContext>(shaders);

	m_rootRenderContext = m_graphicsContext->CreateRenderContext();
	m_currentCamera = std::make_shared<CCamera>(glm::vec3(0.0f, 0.0f, -5.0f));
	// TODO: We need a different way of handling shader selection. Once skeletal meshes are in
	// this manual method is going to grow out of hand.
	m_rootRenderContext->render.default_shader = "unlit_vcol_tex_skel";
	m_rootRenderContext->render.highlight_shader = "constant";
	m_rootRenderContext->render.textureless_shader = "unlit_vcol";
	
	m_rootRenderContext->render.current_camera = m_currentCamera;
	m_rootRenderContext->render.wireframe = false;
	m_rootRenderContext->render.no_blend = false;
	m_rootRenderContext->render.no_cull = false;
	m_rootRenderContext->render.no_texture = false;
	m_rootRenderContext->render.nearClip = 0.1f;
	m_rootRenderContext->render.farClip = 1000.0f;
	m_rootRenderContext->render.no_fog = true;
	m_rootRenderContext->env.fogColor = glm::vec4(1.0f);
	m_rootRenderContext->env.fogNear = 1000.0f - FLT_EPSILON;
	m_rootRenderContext->env.fogFar = 1000.0f;
	m_rootRenderContext->env.clearColor = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);

	float deltaTime;
	float lastFrame = glfwGetTime();
	double worldTime = 0.0;
	while (!m_shouldQuit)
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		if (deltaTime > MAX_DELTA_TIME) deltaTime = MAX_DELTA_TIME;
		lastFrame = currentFrame;
		worldTime += deltaTime;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ProcessInput before NewFrame so we have the newest camera transform
		ProcessInput(m_window.window, deltaTime, worldTime);

		// TODO: better place for frame width/height
		m_rootRenderContext->env.frameHeight = m_window.height;
		m_rootRenderContext->env.frameWidth = m_window.width;
		m_rootRenderContext->NewFrame();
		
		// ProcessGUI first so changes take effect before we Update
		ProcessGUI();

		// Update the animation
		Update(deltaTime, worldTime);
		
		// Draw the mesh to the framebuffer (No viewport window here)
		Render();

		// Draw the gui to the framebuffer
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Display the framebuffer on screen and handle window events
		glfwSwapBuffers(m_window.window);
		glfwPollEvents();

		// Process end of frame stuff (like loading a new file?)
		ProcessDelayed();
	}

	// TODO: Shutdown
}

bool MeshViewer::Init()
{
	// ## INIT GLFW (Window) ##

	if (glfwInit() != GLFW_TRUE)
	{
		glfwTerminate();
		std::cerr << "GLFW INIT FAIL!" << std::endl;
		return false;
	}

	std::cout << "[GS] Compiled with GLFW " << glfwGetVersionString() << std::endl;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
	if (m_settings.USE_ANTIALIASING) glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(m_settings.WIND_WIDTH, m_settings.WIND_HEIGHT, "BBS Mesh Viewer", NULL, NULL);
	m_window.window = window;
	if (window == NULL)
	{
		std::cout << "[GS] Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwSetWindowUserPointer(window, this);
	glfwMakeContextCurrent(window);
	if (!m_settings.USE_VSYNC) glfwSwapInterval(0);

	if (glfwRawMouseMotionSupported()) m_mouse.rawMotionAvailible = true;

	// ## INIT OPENGL ##

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "[GS] Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return false;
	}

	if (GLAD_GL_EXT_texture_filter_anisotropic)
	{
		std::cout << "[GS] Loaded extention GL_EXT_texture_filter_anisotropic!" << std::endl;
		float max_anis;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anis);
		std::cout << "[GS] Max anisotropic samples: " << max_anis << std::endl;
	}

	glViewport(0, 0, m_settings.WIND_WIDTH, m_settings.WIND_HEIGHT);
	m_window.width = m_settings.WIND_WIDTH;
	m_window.height = m_settings.WIND_HEIGHT;

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "[GS] Renderer: " << renderer << std::endl;
	std::cout << "[GS] OpenGL version: " << version << std::endl;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (m_settings.USE_ANTIALIASING) glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glDepthFunc(GL_LEQUAL);

	// ## INIT IMGUI ##

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigDockingWithShift = true;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigWindowsResizeFromEdges = true;
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();
	ImGui::GetStyle().ColorButtonPosition = ImGuiDir_Right;
	ImGui::GetStyle().FrameBorderSize = 1.0f;
	ImGui::GetStyle().WindowRounding = 7.0f;
	ImGui_ImplGlfw_InitForOpenGL(m_window.window, true);
	ImGui_ImplOpenGL3_Init();

	//std::string fontPath = fileManager.GetFontPath("Roboto-Medium.ttf");
	std::string fontPath = m_fileManager->GetResourcePath(EResourceType::RSRC_FONT, "Aldrich-Regular.ttf");
	ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 13.0f);
	assert(font != nullptr);


	return true;
}

void MeshViewer::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	void* windPtr = glfwGetWindowUserPointer(window);
	if (windPtr != nullptr)
	{
		MeshViewer* pThis = reinterpret_cast<MeshViewer*>(windPtr);
		pThis->OnFramebufferSizeChanged(window, width, height);
	}
}

void MeshViewer::OnFramebufferSizeChanged(GLFWwindow* window, int width, int height)
{
	m_window.height = height;
	m_window.width = width;
}

void MeshViewer::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	void* windPtr = glfwGetWindowUserPointer(window);
	if (windPtr != nullptr)
	{
		MeshViewer* pThis = reinterpret_cast<MeshViewer*>(windPtr);
		pThis->OnMouseMoved(window, xpos, ypos);
	}
}

void MeshViewer::OnMouseMoved(GLFWwindow* window, double xpos, double ypos)
{
	if (m_mouse.firstData)
	{
		m_mouse.xPos = xpos;
		m_mouse.yPos = ypos;
		m_mouse.deltaX = 0.0f;
		m_mouse.deltaY = 0.0f;
		m_mouse.firstData = false;
	}

	// TODO: Accumulate deltas then Process them in ProcessInput
	m_mouse.deltaX += xpos - m_mouse.xPos;
	m_mouse.deltaY += m_mouse.yPos - ypos; // reversed since y-coordinates go from bottom to top

	m_mouse.xPos = xpos;
	m_mouse.yPos = ypos;
}

void MeshViewer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	void* windPtr = glfwGetWindowUserPointer(window);
	if (windPtr != nullptr)
	{
		MeshViewer* pThis = reinterpret_cast<MeshViewer*>(windPtr);
		pThis->OnMouseScroll(window, xoffset, yoffset);
	}
}

void MeshViewer::OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	// TODO: Accumulate deltas then Process them in ProcessInput
	m_mouse.deltaScroll += yoffset;
}

// TODO: We could handle mouse button + keyboard events directly.

// TODO: I'd like to get rid of the deltaTime here so we only have it during Update().
// That would mean storing the keyboard inputs somewhere so the camera can get them later.
// Also might be better to push some of the input handling logic down into the camera so it smoothes
// out the differences between different camera types.
void MeshViewer::ProcessInput(GLFWwindow* window, float deltaTime, double worldTime)
{
	auto& imguiIo = ImGui::GetIO();

	if (!imguiIo.WantCaptureMouse)
	{
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
		{
			m_currentCamera->ProcessMouseMovement(m_mouse.deltaX, m_mouse.deltaY);
		}
		m_currentCamera->ProcessMouseScroll(m_mouse.deltaScroll);
	}
	
	m_mouse.deltaX = 0.0f;
	m_mouse.deltaY = 0.0f;
	m_mouse.deltaScroll = 0.0f;

	if (!imguiIo.WantCaptureKeyboard)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			m_shouldQuit = true;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			m_currentCamera->ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			m_currentCamera->ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			m_currentCamera->ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			m_currentCamera->ProcessKeyboard(RIGHT, deltaTime);

		// TODO: Camera movement speed setting
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			m_currentCamera->MovementMultiplier = 2.0f;
		else
			m_currentCamera->MovementMultiplier = 1.0f;
	}

	if (glfwWindowShouldClose(m_window.window))
		m_shouldQuit = true;
}

void MeshViewer::Update(float deltaTime, double worldTime)
{
	for (auto& tex : m_textures)
	{
		if (tex != nullptr)
			tex->Update(deltaTime, worldTime);
	}

	if (m_guiAnim != nullptr) m_guiAnim->Update(deltaTime, worldTime);

	// TODO: I think CSkelModelObject would benifit from an Update function
	if (m_model != nullptr)
	{
		m_model->UpdateTextureOffsets();

		if (m_model->animDriver != nullptr)
			m_model->animDriver->Update(deltaTime, worldTime);

		//m_rootRenderContext->AddToDrawList(LAYER_STATIC, m_model);
		m_rootRenderContext->AddToDrawList(LAYER_DYNAMIC, m_model);
	}

	if (drawSkel) DrawSkeleton();
}

void MeshViewer::Render()
{
		m_rootRenderContext->Render();
}

void MeshViewer::ScheduleDelayedProcess(DelayedFunc func)
{
	m_scheduledDelays.push_back(func);
}

void MeshViewer::ProcessDelayed()
{
	for (auto func : m_scheduledDelays)
		std::invoke(func, this);

	m_scheduledDelays.clear();
}

void MeshViewer::DrawSkeleton()
{
	if (m_model == nullptr)
		return;
	auto pose = m_model->animDriver->GetPose();
	for (auto& bone : pose)
	{
		if (bone.parentIdx != -1)
		{
			auto bonePos = glm::vec3(pose[bone.idx].transform[3]);
			auto parentPos = glm::vec3(pose[bone.parentIdx].transform[3]);
			DebugDraw::DebugLine(*m_rootRenderContext, parentPos, bonePos, glm::vec3(1.0f, 0.0f, 0.0f));
		}
	}
}