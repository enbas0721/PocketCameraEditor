/***********************************************************************************
 * PocketCameraEditor.cpp : "main" function of the PocketCameraEditor application.
 *
 * Starts the libraries up, runs the frame loop, and hands each frame to the UI.
 * The windows themselves live in AppUI.cpp.
 ***********************************************************************************/
#include "AppContext.h"
#include "AppState.h"
#include "AppUI.h"

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int main()
{
	// Declaration order is shutdown order, reversed. Everything that touches the
	// GL context must be declared after the window that owns it.
	GlfwContext glfwContext;
	if (!glfwContext.IsValid())
		return 1;

	NfdContext nfdContext;
	if (!nfdContext.IsValid())
		return 1;

	AppWindow window(1280, 720, "PocketCameraEditor");
	if (!window.IsValid())
		return 1;

	ImGuiRuntime imguiRuntime(window.Get(), "#version 330");

	// AppState owns the textures, so it has to be destroyed before ImGuiRuntime
	// and AppWindow tear the GL context down.
	AppState app;

	// The classic Windows 98 desktop teal.
	constexpr float kClearColor[4] = { 0.0f, 0.502f, 0.502f, 1.0f };

	while (!glfwWindowShouldClose(window.Get()))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DrawAppUI(app);

		ImGui::Render();

		int displayWidth = 0;
		int displayHeight = 0;
		glfwGetFramebufferSize(window.Get(), &displayWidth, &displayHeight);
		glViewport(0, 0, displayWidth, displayHeight);
		glClearColor(kClearColor[0], kClearColor[1], kClearColor[2], kClearColor[3]);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window.Get());
	}

	return 0;
}
