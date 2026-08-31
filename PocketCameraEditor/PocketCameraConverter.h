// PocketCameraConverter.h
// Loads a GameBoy PocketCamera .sav file and decodes its image data into an RGBA buffer.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

class PocketCameraConverter
{
public:
	static constexpr int kImageWidth = 128;
	static constexpr int kImageHeight = 112;
	static constexpr int kMaxImageCount = 30;
	static constexpr size_t kExpectedFileSize = 128 * 1024; // 128KB
	static constexpr uint32_t kStateVectorOffset = 0x11B2; // Offset of the state vector in the .sav file.
	static constexpr uint32_t kMaxSlotDisplayOrder = 0x1D; // Maximum value for a slot's display order in the state vector.

	// Pixel buffer of width * height * 4 (RGBA) bytes.
	using RgbaBuffer = std::vector<uint8_t>;
	using IndexBuffer = std::vector<uint8_t>;

	// Loads the .sav file into memory. Fails if the size is not exactly 128KB.
	bool LoadFromFile(const std::string& path);

	// Whether a file has been successfully loaded.
	bool IsLoaded() const { return !m_data.empty(); }

	// Whether the slot at 'index' is active on the PocketCamera.
	// Deleted/unused slots still hold valid image data.
	bool IsSlotActive(int index) const;

	// Returns the display order of the slot at 'index' (0-29).
	int GetSlotDisplayOrder(int index) const;

	// Decodes image `index` (0-29) into a 128x112 color index buffer.
	IndexBuffer DecodeIndices(int index) const;

private:
	static constexpr uint32_t ImageOffset(int index)
	{
		// 0x0000-0x0FFF is the sensor's latest frame (exchange buffer),
		// so stored images start at 0x2000.
		return 0x2000 + static_cast<uint32_t>(index) * 0x1000;
	}

	std::vector<uint8_t> m_data;
};
