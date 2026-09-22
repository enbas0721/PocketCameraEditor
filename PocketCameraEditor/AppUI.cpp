// AppUI.cpp
#include "AppUI.h"

#include "AppState.h"
#include "ImageExporter.h"
#include "Palette.h"
#include "PocketCameraConverter.h"

#include "imgui.h"
#include "nfd.hpp"

#include <cstdio>
#include <filesystem>
#include <format>
#include <vector>

namespace
{
	constexpr int kThumbnailsPerRow = 3;

	// NFDe hands back UTF-8, but std::filesystem::path reads a plain char* using
	// the system's narrow code page. Going through char8_t keeps non-ASCII paths
	// intact.
	std::filesystem::path ToPath(const char* utf8)
	{
		return std::filesystem::path(reinterpret_cast<const char8_t*>(utf8));
	}

	// Builds the RGBA image that would be written to disk for one slot.
	// Returns an empty image when the upscale fails.
	ExportImage BuildExportImage(const Slot& slot, int scale)
	{
		const std::vector<uint8_t> rgba = ApplyPalette(slot.indices, slot.palette);
		return UpscaleNearest(rgba,
		                      PocketCameraConverter::kImageWidth,
		                      PocketCameraConverter::kImageHeight,
		                      scale);
	}

	void OpenSavFile(AppState& app)
	{
		NFD::UniquePath inPath;
		nfdfilteritem_t filters[1] = { { "GameBoy Camera Save", "sav" } };

		const nfdresult_t result = NFD::OpenDialog(inPath, filters, 1);
		if (result == NFD_OKAY)
		{
			if (app.library.Load(ToPath(inPath.get())))
			{
				app.currentIndex = 0;
				app.currentFilePath = inPath.get();
				app.statusMessage.clear();
			}
			else
			{
				app.statusMessage = std::format("Error: failed to load {}", inPath.get());
			}
		}
		else if (result == NFD_ERROR)
		{
			app.statusMessage = std::format("Error: {}", NFD::GetError());
		}
		// NFD_CANCEL: the user changed their mind, nothing to report.
	}

	void SetAllSelected(AppState& app, bool selected)
	{
		for (int i = 0; i < app.library.SlotCount(); ++i)
			app.library.GetSlot(i).isSelected = selected;
	}

	// One cell of the thumbnail grid: the image with a checkbox laid over it.
	void DrawSlotThumbnail(AppState& app, int index)
	{
		Slot& slot = app.library.GetSlot(index);

		// The group keeps the image and its overlapping checkbox together as a
		// single layout item, so SameLine() still lines the grid up afterwards.
		ImGui::BeginGroup();
		{
			const ImVec2 slotPos = ImGui::GetCursorScreenPos();

			char buttonId[16];
			snprintf(buttonId, std::size(buttonId), "Image %d", index);

			// Lets the checkbox submitted below win the mouse over this button.
			ImGui::SetNextItemAllowOverlap();
			if (ImGui::ImageButton(buttonId,
			                       ImTextureID(slot.texture.GetId()),
			                       ImVec2(PocketCameraConverter::kImageWidth,
			                              PocketCameraConverter::kImageHeight)))
			{
				app.currentIndex = index;
			}

			const ImVec2 imageMin = ImGui::GetItemRectMin();
			const ImVec2 imageMax = ImGui::GetItemRectMax();

			char checkboxId[16];
			snprintf(checkboxId, std::size(checkboxId), "##chk%d", index);
			ImGui::SetCursorScreenPos(ImVec2(slotPos.x + 4.0f, slotPos.y + 4.0f));
			ImGui::Checkbox(checkboxId, &slot.isSelected);

			if (app.currentIndex == index)
			{
				ImGui::GetWindowDrawList()->AddRect(
					imageMin, imageMax, IM_COL32(66, 150, 250, 255), 0.0f, 0, 5.0f);
			}
		}
		ImGui::EndGroup();
	}

	void DrawLibraryWindow(AppState& app)
	{
		ImGui::Begin("Library");
		{
			if (ImGui::Button("Open File"))
				OpenSavFile(app);

			ImGui::Text("Current File: %s", app.currentFilePath.c_str());

			if (app.library.IsLoaded())
			{
				if (ImGui::Button("Select All"))
					SetAllSelected(app, true);
				ImGui::SameLine();
				if (ImGui::Button("Deselect All"))
					SetAllSelected(app, false);

				ImGui::Checkbox("Hide Deleted", &app.hideDeleted);

				// Counts the thumbnails actually drawn: using the slot index for the
				// row break would leave gaps whenever a slot is filtered out.
				int placed = 0;
				for (int i = 0; i < app.library.SlotCount(); ++i)
				{
					if (app.hideDeleted && !app.library.IsSlotActive(i))
						continue;

					DrawSlotThumbnail(app, i);

					if (placed % kThumbnailsPerRow != kThumbnailsPerRow - 1)
						ImGui::SameLine();
					++placed;
				}
			}
		}
		ImGui::End();
	}

	void DrawViewerWindow(AppState& app)
	{
		ImGui::Begin("Viewer");
		{
			const ImVec2 size(
				static_cast<float>(PocketCameraConverter::kImageWidth * app.exportScale),
				static_cast<float>(PocketCameraConverter::kImageHeight * app.exportScale));
			ImGui::Image(ImTextureID(app.library.GetSlot(app.currentIndex).texture.GetId()), size);
		}
		ImGui::End();
	}

	void DrawInfoWindow(AppState& app)
	{
		ImGui::Begin("Info");
		{
			ImGui::Text("Image slot: %d", app.currentIndex + 1);

			const int displayOrder = app.library.GetSlotDisplayOrder(app.currentIndex);
			if (displayOrder < 0)
				ImGui::Text("Image slot display order: deleted");
			else
				ImGui::Text("Image slot display order: %d", displayOrder + 1);
		}
		ImGui::End();
	}

	void DrawPaletteWindow(AppState& app)
	{
		ImGui::Begin("Palette");
		{
			Slot& slot = app.library.GetSlot(app.currentIndex);
			bool changed = false;

			if (ImGui::BeginCombo("Preset", kPresets[slot.presetId].name))
			{
				for (int i = 0; i < static_cast<int>(std::size(kPresets)); ++i)
				{
					const bool isSelected = (slot.presetId == i);
					if (ImGui::Selectable(kPresets[i].name, isSelected))
					{
						slot.presetId = i;
						slot.palette = kPresets[i].palette;
						changed = true;
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			// |= rather than ||: the latter would short-circuit and skip submitting
			// the remaining pickers, making them vanish from the window.
			constexpr ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel;
			changed |= ImGui::ColorEdit3("Shade 0 (lightest)", slot.palette.colors[0], flags);
			changed |= ImGui::ColorEdit3("Shade 1",            slot.palette.colors[1], flags);
			changed |= ImGui::ColorEdit3("Shade 2",            slot.palette.colors[2], flags);
			changed |= ImGui::ColorEdit3("Shade 3 (darkest)",  slot.palette.colors[3], flags);

			// Only the edited slot needs re-uploading; every slot has its own palette.
			if (changed)
				app.library.RefreshTexture(app.currentIndex);
		}
		ImGui::End();
	}

	void ExportCurrentSlot(AppState& app)
	{
		const std::string defaultName =
			std::format("slot_{:02d}_x{}.png", app.currentIndex + 1, app.exportScale);

		NFD::UniquePath outPath;
		nfdfilteritem_t filters[1] = { { "PNG Image", "png" } };

		const nfdresult_t result =
			NFD::SaveDialog(outPath, filters, 1, nullptr, defaultName.c_str());

		if (result == NFD_CANCEL)
			return;
		if (result == NFD_ERROR)
		{
			app.statusMessage = std::format("Error: {}", NFD::GetError());
			return;
		}

		const ExportImage image =
			BuildExportImage(app.library.GetSlot(app.currentIndex), app.exportScale);
		if (image.pixels.empty())
		{
			app.statusMessage = std::format("Error: failed to build image for {}", outPath.get());
			return;
		}

		if (SavePng(ToPath(outPath.get()), image))
			app.statusMessage = std::format("Saved {}", outPath.get());
		else
			app.statusMessage = std::format("Error: failed to write {}", outPath.get());
	}

	void ExportSelectedSlots(AppState& app)
	{
		NFD::UniquePath outPath;
		const nfdresult_t result = NFD::PickFolder(outPath);

		if (result == NFD_CANCEL)
			return;
		if (result == NFD_ERROR)
		{
			app.statusMessage = std::format("Error: {}", NFD::GetError());
			return;
		}

		const std::filesystem::path folder = ToPath(outPath.get());
		int selectedCount = 0;
		int savedCount = 0;

		// A single failure should not abandon the rest; the tally is reported at the end.
		for (int i = 0; i < app.library.SlotCount(); ++i)
		{
			const Slot& slot = app.library.GetSlot(i);
			if (!slot.isSelected)
				continue;

			++selectedCount;

			const ExportImage image = BuildExportImage(slot, app.exportScale);
			if (image.pixels.empty())
				continue;

			const std::filesystem::path savePath =
				folder / std::format("slot_{:02d}_x{}.png", i + 1, app.exportScale);
			if (SavePng(savePath, image))
				++savedCount;
		}

		app.statusMessage = std::format("Saved {}/{} images to {}",
		                                savedCount, selectedCount, outPath.get());
	}

	void DrawExportWindow(AppState& app)
	{
		ImGui::Begin("Export Setting");
		{
			ImGui::SliderInt("Export Scale", &app.exportScale, 1, 8);
			ImGui::Text("Output size: %d x %d",
			            PocketCameraConverter::kImageWidth * app.exportScale,
			            PocketCameraConverter::kImageHeight * app.exportScale);

			if (ImGui::Button("Save PNG"))
				ExportCurrentSlot(app);
			ImGui::SameLine();
			if (ImGui::Button("Save All Selected"))
				ExportSelectedSlots(app);

			if (!app.statusMessage.empty())
				ImGui::TextWrapped("%s", app.statusMessage.c_str());
		}
		ImGui::End();
	}
}

void DrawAppUI(AppState& app)
{
	// The Library window owns the Open button, so it is shown even with no file.
	DrawLibraryWindow(app);

	// The rest read GetSlot(currentIndex), which needs a loaded file.
	if (!app.library.IsLoaded())
		return;

	DrawViewerWindow(app);
	DrawInfoWindow(app);
	DrawPaletteWindow(app);
	DrawExportWindow(app);
}
