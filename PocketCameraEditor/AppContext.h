// AppContext.h
// RAII wrappers around the libraries that need paired init/shutdown calls.
//
// Declare them in dependency order inside main(): C++ destroys locals in reverse
// order of declaration, so the shutdown sequence is guaranteed by the language
// rather than by remembering to write the calls in the right places.
//
// This matters for Texture in particular: its destructor calls glDeleteTextures,
// so every Texture must die while the GL context is still alive.
#pragma once

struct GLFWwindow;

// glfwInit / glfwTerminate.
class GlfwContext
{
public:
	GlfwContext();
	~GlfwContext();
	GlfwContext(const GlfwContext&) = delete;
	GlfwContext& operator=(const GlfwContext&) = delete;

	bool IsValid() const { return m_initialized; }

private:
	bool m_initialized = false;
};

// NFD::Init / NFD::Quit.
class NfdContext
{
public:
	NfdContext();
	~NfdContext();
	NfdContext(const NfdContext&) = delete;
	NfdContext& operator=(const NfdContext&) = delete;

	bool IsValid() const { return m_initialized; }

private:
	bool m_initialized = false;
};

// Creates the window and its OpenGL context, and loads the GL functions.
class AppWindow
{
public:
	AppWindow(int width, int height, const char* title);
	~AppWindow();
	AppWindow(const AppWindow&) = delete;
	AppWindow& operator=(const AppWindow&) = delete;

	bool IsValid() const { return m_window != nullptr; }
	GLFWwindow* Get() const { return m_window; }

private:
	GLFWwindow* m_window = nullptr;
};

// ImGui context plus its GLFW/OpenGL3 backends.
class ImGuiRuntime
{
public:
	ImGuiRuntime(GLFWwindow* window, const char* glslVersion);
	~ImGuiRuntime();
	ImGuiRuntime(const ImGuiRuntime&) = delete;
	ImGuiRuntime& operator=(const ImGuiRuntime&) = delete;
};
