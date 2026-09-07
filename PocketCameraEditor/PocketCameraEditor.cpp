/***********************************************************************************
 * PocketCameraEditor.cpp : "main" function of the PocketCameraEditor application.
 ***********************************************************************************/
#include "PocketCameraConverter.h"
#include "Texture.h"
#include "Palette.h"
#include "ImageExporter.h"
#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <cstdio>
#include <vector>
#include <string>
#include <cassert>


char saveStatus[256] = "";

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void RefreshTextures(std::vector<Texture>& textures, const std::vector<PocketCameraConverter::IndexBuffer>& textureIndices, const Palette& palette)
{
	assert(textures.size() == textureIndices.size());

	for (size_t i = 0; i < textures.size(); ++i)
	{
		textures[i].Upload(ApplyPalette(textureIndices[i], palette), PocketCameraConverter::kImageWidth, PocketCameraConverter::kImageHeight);
	}
}

int main()
{
	int currentIndex = 0;
	int saveScale = 4;
	const ImVec2 kViewerScaledSize = ImVec2(PocketCameraConverter::kImageWidth * 4, PocketCameraConverter::kImageHeight * 4);
	const char* filepath = "../../tmp/pcktcmr-test2.sav";

	bool hide_deleted = false;

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
		std::vector<PocketCameraConverter::IndexBuffer> textureIndices;
		textureIndices.reserve(PocketCameraConverter::kMaxImageCount);

		std::vector<Texture> textures;
		textures.reserve(PocketCameraConverter::kMaxImageCount);

		int presetId = 0;
		Palette currentPalette = kPresets[presetId].palette;
		// Refresh at first frame.
		bool isNeedRefresh = true;

		for (int i = 0; i < PocketCameraConverter::kMaxImageCount; ++i)
		{
			textureIndices.push_back(converter.DecodeIndices(i));
			textures.emplace_back();
		}

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();

			ImGui::NewFrame();

			if (isNeedRefresh)
			{
				RefreshTextures(textures, textureIndices, currentPalette);
				isNeedRefresh = false;
			}

			ImGui::Begin("Library");
			{
				int placed = 0;
				char textImageButton[16];

				ImGui::Text("sav file Path: %s", filepath);
				ImGui::Checkbox("Hide Deleted", &hide_deleted);
				for (int i = 0; i < PocketCameraConverter::kMaxImageCount; ++i)
				{
					// Skip rendering invalid images.
					if (hide_deleted && !converter.IsSlotActive(i))
					{
						continue;
					}
					snprintf(textImageButton, std::size(textImageButton), "Image %d", i);
					if (ImGui::ImageButton(textImageButton, ImTextureID(textures[i].GetId()), ImVec2(PocketCameraConverter::kImageWidth, PocketCameraConverter::kImageHeight)))
					{
						currentIndex = i;
					}
					if (currentIndex == i)
					{
						const ImVec2 imageButtonMin = ImGui::GetItemRectMin();
						const ImVec2 imageButtonMax = ImGui::GetItemRectMax();
						ImDrawList* drawList = ImGui::GetWindowDrawList();
						drawList->AddRect(imageButtonMin, imageButtonMax, IM_COL32(66, 150, 250, 255), 0.0f, 0, 5.0f);
					}
					if (placed % 3 != 2)
					{
						ImGui::SameLine();
					}
					++placed;
				}
			}
			ImGui::End();

			// Show the image viewer window with the current image index and scale.
			ImGui::Begin("Viewer");
			{
				ImGui::Image(ImTextureID(textures[currentIndex].GetId()), kViewerScaledSize);
			}
			ImGui::End();

			ImGui::Begin("Info");
			{
				ImGui::Text("Image slot: %d", currentIndex + 1);
				int displayOrder = converter.GetSlotDisplayOrder(currentIndex);
				if (displayOrder == -1)
				{
					ImGui::Text("Image slot display order: deleted");
				}
				else
				{
					ImGui::Text("Image slot display order: %d", displayOrder + 1);
				}
			}
			ImGui::End();

			ImGui::Begin("Palette");
			{
				if (ImGui::BeginCombo("Preset", kPresets[presetId].name))
				{
					for (int i = 0; i < static_cast<int>(std::size(kPresets)); ++i)
					{
						const bool isSelected = (presetId == i);
						if (ImGui::Selectable(kPresets[i].name, isSelected))
						{
							presetId = i;
							currentPalette = kPresets[presetId].palette;
							isNeedRefresh = true;
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				isNeedRefresh |= ImGui::ColorEdit3("Shade 0 (lightest)", currentPalette.colors[0]);
				isNeedRefresh |= ImGui::ColorEdit3("Shade 1", currentPalette.colors[1]);
				isNeedRefresh |= ImGui::ColorEdit3("Shade 2", currentPalette.colors[2]);
				isNeedRefresh |= ImGui::ColorEdit3("Shade 3 (darkest)", currentPalette.colors[3]);
			}
			ImGui::End();

			ImGui::Begin("Export Setting");
			{
				ImGui::SliderInt("Export Scale", &saveScale, 1, 8);
				if (ImGui::Button("Save PNG"))
				{
					// Save the image.
					char savePath[256];
					snprintf(savePath, sizeof(savePath), "C:/tmp/slot_%02d_x%d.png", currentIndex + 1, saveScale);

					std::vector<uint8_t> saveData = ApplyPalette(textureIndices[currentIndex], currentPalette);
					ExportImage finalImage = UpscaleNearest(
						saveData,
						PocketCameraConverter::kImageWidth,
						PocketCameraConverter::kImageHeight,
						saveScale
					);
					if (!finalImage.pixels.empty()) {
						if (!SavePng(savePath, finalImage)) {
							snprintf(saveStatus, sizeof(saveStatus), "Error: Failed to save PNG file: %s", savePath);
						}
						else
						{
							snprintf(saveStatus, sizeof(saveStatus), "Saved PNG file: %s", savePath);
						}
					}
					else {
						snprintf(saveStatus, sizeof(saveStatus), "Error: Failed to save PNG file: %s", savePath);
					}
				}
				if (saveStatus[0] != '\0')
				{
					ImGui::Text("%s", saveStatus);
				}

			}
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