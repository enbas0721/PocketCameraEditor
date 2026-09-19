/***********************************************************************************
 * PocketCameraEditor.cpp : "main" function of the PocketCameraEditor application.
 ***********************************************************************************/
#include "PocketCameraConverter.h"
#include "Texture.h"
#include "Palette.h"
#include "ImageExporter.h"
#include "PhotoLibrary.h"
#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "nfd.hpp"

#include <cstdio>
#include <vector>
#include <string>
#include <filesystem>

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main()
{
	int currentIndex = 0;
	int saveScale = 4;
	const ImVec2 kViewerScaledSize = ImVec2(PocketCameraConverter::kImageWidth * 4, PocketCameraConverter::kImageHeight * 4);
	
	PhotoLibrary library;

	char saveStatus[256] = "";
	std::string currentFilePath = "No file loaded.";

	bool hide_deleted = false;

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	nfdresult_t result = NFD::Init();
	if (result != NFD_OKAY)
	{
		fprintf(stderr, "Failed to initialize NFD!\n");
		glfwTerminate();
		return 1;
	}

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

	// Prepare the textures and index buffers for all images.
	{
		/**************************************
		 * Main loop
		 **************************************/
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();

			ImGui::NewFrame();

			ImGui::Begin("Library");
			{
				int placed = 0;
				char textImageButton[16];

				if (ImGui::Button("Open File"))
				{
					NFD::UniquePath inPath;
					nfdfilteritem_t filters[1] = {{ "GameBoy Camera Save", "sav"}};
					nfdresult_t result = NFD::OpenDialog(inPath, filters, 1);
					if (result == NFD_OKAY)
					{
						std::filesystem::path loadPath(reinterpret_cast<const char8_t*>(inPath.get()));
						if (!library.Load(loadPath))
						{
							fprintf(stderr, "Failed to load PocketCamera file!\n");
						} else {
							currentIndex = 0;
							currentFilePath = inPath.get();
						}
						
					}
					else if (result == NFD_CANCEL)
					{
						// User canceled the open dialog, do nothing.
					}
					else
					{
						fprintf(stderr, "Error: %s\n", NFD::GetError());
					}
				}
				ImGui::Text("Current File: %s", currentFilePath.c_str());
				
				if (library.IsLoaded()) {
					ImGui::Checkbox("Hide Deleted", &hide_deleted);
					for (int i = 0; i < library.SlotCount(); ++i)
					{
						Slot& slot = library.GetSlot(i);
						// Skip rendering invalid images.
						if (hide_deleted && !library.IsSlotActive(i))
						{
							continue;
						}
						snprintf(textImageButton, std::size(textImageButton), "Image %d", i);
						if (ImGui::ImageButton(textImageButton, ImTextureID(slot.texture.GetId()), ImVec2(PocketCameraConverter::kImageWidth, PocketCameraConverter::kImageHeight)))
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
			}
			ImGui::End();

			if (library.IsLoaded())
			{
				// Show the image viewer window with the current image index and scale.
				ImGui::Begin("Viewer");
				{
					ImGui::Image(ImTextureID(library.GetSlot(currentIndex).texture.GetId()), kViewerScaledSize);
				}
				ImGui::End();

				ImGui::Begin("Info");
				{
					ImGui::Text("Image slot: %d", currentIndex + 1);
					int displayOrder = library.GetSlotDisplayOrder(currentIndex);
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
					Slot& slot = library.GetSlot(currentIndex);
					bool changed = false;
					if (ImGui::BeginCombo("Preset", kPresets[slot.presetId].name))
					{
						for (int i = 0; i < static_cast<int>(std::size(kPresets)); ++i)
						{
							const bool isSelected = (slot.presetId == i);
							if (ImGui::Selectable(kPresets[i].name, isSelected))
							{
								slot.presetId = i;
								slot.palette = kPresets[slot.presetId].palette;
								changed = true;
							}
							if (isSelected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}
					changed |= ImGui::ColorEdit3("Shade 0 (lightest)", slot.palette.colors[0]);
					changed |= ImGui::ColorEdit3("Shade 1", slot.palette.colors[1]);
					changed |= ImGui::ColorEdit3("Shade 2", slot.palette.colors[2]);
					changed |= ImGui::ColorEdit3("Shade 3 (darkest)", slot.palette.colors[3]);

					if (changed)
					{
						library.RefreshTexture(currentIndex);
					}
				}
				ImGui::End();

				ImGui::Begin("Export Setting");
				{
					ImGui::SliderInt("Export Scale", &saveScale, 1, 8);
					if (ImGui::Button("Save PNG"))
					{
						// Save the image.
						char saveDefaultPath[256];
						snprintf(saveDefaultPath, sizeof(saveDefaultPath), "slot_%02d_x%d.png", currentIndex + 1, saveScale);

						NFD::UniquePath outPath;
						nfdfilteritem_t filters[1] = { { "PNG Image", "png"} };

						nfdresult_t result = NFD::SaveDialog(outPath, filters, 1, nullptr, saveDefaultPath);

						if (result == NFD_OKAY)
						{
							Slot& slot = library.GetSlot(currentIndex);
							// Make save Image.
							std::vector<uint8_t> saveData = ApplyPalette(slot.indices, slot.palette);
							ExportImage finalImage = UpscaleNearest(
								saveData,
								PocketCameraConverter::kImageWidth,
								PocketCameraConverter::kImageHeight,
								saveScale
							);
							if (!finalImage.pixels.empty()) {
								if (!SavePng(outPath.get(), finalImage)) {
									snprintf(saveStatus, sizeof(saveStatus), "Error: Failed to Save Image: %s", outPath.get());
								}
								else
								{
									snprintf(saveStatus, sizeof(saveStatus), "Saved PNG file: %s", outPath.get());
								}
							}
							else {
								snprintf(saveStatus, sizeof(saveStatus), "Error: Failed to Convert Image: %s", outPath.get());
							}
						}
						else if (result == NFD_CANCEL)
						{
							// User cancesled the save dialog, do nothing.
						}
						else if (result == NFD_ERROR)
						{
							snprintf(saveStatus, sizeof(saveStatus), "Error: %s", NFD::GetError());
						}
					}
					if (saveStatus[0] != '\0')
					{
						ImGui::Text("%s", saveStatus);
					}

				}
				ImGui::End();
			}

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
	
	NFD::Quit();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}