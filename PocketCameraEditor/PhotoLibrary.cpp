#include "PhotoLibrary.h"

PhotoLibrary::PhotoLibrary()
{
}

bool PhotoLibrary::Load(const std::filesystem::path& path)
{
	if (!m_converter.LoadFromFile(path))
	{
		return false;
	}

	m_slots.resize(PocketCameraConverter::kMaxImageCount);

	for (int i = 0; i < PocketCameraConverter::kMaxImageCount; ++i)
	{
		m_slots[i].presetId = 0;
		m_slots[i].indices = m_converter.DecodeIndices(i);
		m_slots[i].palette = kPresets[0].palette;
	}

	for (int i = 0; i < PocketCameraConverter::kMaxImageCount; ++i)
	{
		RefreshTexture(i);
	}
	return true;
}

bool PhotoLibrary::IsLoaded() const
{
	return m_converter.IsLoaded();
}

int PhotoLibrary::SlotCount() const
{
	return static_cast<int>(m_slots.size());
}

Slot& PhotoLibrary::GetSlot(int index)
{
	return m_slots[index];
}

bool PhotoLibrary::IsSlotActive(int index) const
{
	return m_converter.IsSlotActive(index);
}

int PhotoLibrary::GetSlotDisplayOrder(int index)
{
	return m_converter.GetSlotDisplayOrder(index);
}

void PhotoLibrary::RefreshTexture(int index)
{
	m_slots[index].texture.Upload(ApplyPalette(m_slots[index].indices, m_slots[index].palette), PocketCameraConverter::kImageWidth, PocketCameraConverter::kImageHeight);
}