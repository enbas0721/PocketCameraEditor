#pragma once
#include <vector>

#include "PocketCameraConverter.h"
#include "Texture.h"
#include "Palette.h"

#include <filesystem>

struct Slot
{
	PocketCameraConverter::IndexBuffer indices;
	Texture texture;
	Palette palette;
	int presetId = 0;
};

class PhotoLibrary
{
public:
	PhotoLibrary();
	bool Load(const std::filesystem::path& path);
	bool IsLoaded() const;
	int SlotCount() const;
	Slot& GetSlot(int index);
	bool IsSlotActive(int index) const;
	int GetSlotDisplayOrder(int index);
	void RefreshTexture(int index);
private:
	PocketCameraConverter m_converter;
	std::vector<Slot> m_slots;
};

