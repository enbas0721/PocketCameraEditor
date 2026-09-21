// AppContext.cpp
#include "AppContext.h"

#include "Win98Style.h"

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "nfd.hpp"

#include <cstdio>

namespace
{
	void GlfwErrorCallback(int error, const char* description)
	{
		fprintf(stderr, "Glfw Error %d: %s\n", error, description);
	}
}

GlfwContext::GlfwContext()
{
	glfwSetErrorCallback(GlfwErrorCallback);
	m_initialized = (glfwInit() != 0);
	if (!m_initialized)
		fprintf(stderr, "Failed to initialize GLFW!\n");
}

GlfwContext::~GlfwContext()
{
	if (m_initialized)
		glfwTerminate();
}

NfdContext::NfdContext()
{
	m_initialized = (NFD::Init() == NFD_OKAY);
	if (!m_initialized)
		fprintf(stderr, "Failed to initialize NFD!\n");
}

NfdContext::~NfdContext()
{
	if (m_initialized)
		NFD::Quit();
}

AppWindow::AppWindow(int width, int height, const char* title)
{
	// OpenGL 3.3 Core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (m_window == nullptr)
	{
		fprintf(stderr, "Failed to create a window!\n");
		return;
	}

	glfwMakeContextCurrent(m_window);

	// The loader needs a current context to resolve function pointers.
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		glfwDestroyWindow(m_window);
		m_window = nullptr;
		return;
	}

	glfwSwapInterval(1); // vsync
}

AppWindow::~AppWindow()
{
	if (m_window != nullptr)
		glfwDestroyWindow(m_window);
}

ImGuiRuntime::ImGuiRuntime(GLFWwindow* window, const char* glslVersion)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ApplyWin98Style();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);
}

ImGuiRuntime::~ImGuiRuntime()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
