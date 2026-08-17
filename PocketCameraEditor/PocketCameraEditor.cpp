/***********************************************************************************
 * PocketCameraEditor.cpp : "main" function of the PocketCameraEditor application.
 ***********************************************************************************/
#include "PocketCameraConverter.h"
#include "Texture.h"
#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdio>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
    int i;
    int currentIndex = 0;
    int scale = 4;
    ImVec2 scaledSize = ImVec2(PocketCameraConverter::kImageWidth * scale, PocketCameraConverter::kImageHeight * scale);
    const char* filepath = "../../tmp/pcktcmr-test2.sav";

    int placed = 0;
    bool show_deleted = true;

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // OpenGL 3.3 Core
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "PocketCameraEditor", nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        glfwTerminate();
        return 1;
    }
    glfwSwapInterval(1); // vsync
    
    IMGUI_CHECKVERSION();
    // ImGui::CreateContext() creates a new Dear ImGui context. This function must be called before using any Dear ImGui functions. 
    // It initializes the internal state of Dear ImGui and prepares it for use. 
    // The context is stored in a global variable, and you can retrieve it later using ImGui::GetCurrentContext() if needed.
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    
    ImGui::StyleColorsDark();
    
    // Function for Initializeing ImGui backends.
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Convert a PocketCamera .sav file to RGBA buffers.
    PocketCameraConverter converter;
    // [NOTE] This is hardcoded for testing purposes. This may be changed.
    if (!converter.LoadFromFile(filepath))
    {
        fprintf(stderr, "Failed to load PocketCamera file!\n");
        // [NOTE] This should not be fatal, but for now we will just exit.
        glfwTerminate();
        return 1;

    }
    {
        std::vector<Texture> textures;
        textures.reserve(PocketCameraConverter::kMaxImageCount);

        for (i = 0; i < PocketCameraConverter::kMaxImageCount; ++i)
        {
            textures.emplace_back();
            textures[i].Upload(converter.DecodeImage(i), PocketCameraConverter::kImageWidth, PocketCameraConverter::kImageHeight);
        }

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();

            ImGui::NewFrame();

            ImGui::Begin("Library");
            ImGui::Text("sav file Path: %s", filepath);
			ImGui::Checkbox("Show Deleted", &show_deleted);
            for (i = 0; i < PocketCameraConverter::kMaxImageCount; ++i)
            {
				if (!show_deleted && !converter.IsSlotActive(i))
				{
                    continue;
				}

                if (ImGui::ImageButton(("Image " + std::to_string(i)).c_str(), ImTextureID(textures[i].GetId()), ImVec2(PocketCameraConverter::kImageWidth, PocketCameraConverter::kImageHeight)))
                {
                    currentIndex = i;
                }
                if (placed % 3 != 2)
                {
                    ImGui::SameLine();
                }
                ++placed;
            }
            placed = 0;
            ImGui::End();
            
            // Show the image viewer window with the current image index and scale.
            ImGui::Begin("Viewer");

            if (ImGui::SliderInt("Scale", &scale, 1, 8))
            {
                scaledSize = ImVec2(PocketCameraConverter::kImageWidth * scale, PocketCameraConverter::kImageHeight * scale);
            }
            ImGui::Image(ImTextureID(textures[currentIndex].GetId()), scaledSize);
            ImGui::End();

            ImGui::Render();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}