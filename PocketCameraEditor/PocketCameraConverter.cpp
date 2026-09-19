// PocketCameraConverter.cpp
#include "PocketCameraConverter.h"

#include <fstream>

bool PocketCameraConverter::LoadFromFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		m_data.clear();
		return false;
	}

	const auto size = static_cast<size_t>(file.tellg());
	if (size != kExpectedFileSize)
	{
		m_data.clear();
		return false;
	}

	m_data.resize(size);
	file.seekg(0);
	file.read(reinterpret_cast<char*>(m_data.data()), static_cast<std::streamsize>(size));

	if (!file)
	{
		m_data.clear();
		return false;
	}
	return true;
}

bool PocketCameraConverter::IsSlotActive(int index) const
{
	if (index < 0 || index >= kMaxImageCount)
		return false;
	if (IsLoaded())
	{
		// Check the state vector for the slot's active status
		const uint32_t stateOffset = kStateVectorOffset + index;
		if (stateOffset < m_data.size())
		{
			// In the state vector,  > 0x1D indicates an inactive slot.
			return m_data[stateOffset] <= kMaxSlotDisplayOrder;
		}
	}
	return false;
}

int PocketCameraConverter::GetSlotDisplayOrder(int index) const
{
	if (!IsSlotActive(index))
	{
		return -1; // Return an invalid value for inactive slots
	}

	const uint32_t stateOffset = kStateVectorOffset + index;
	if (stateOffset < m_data.size())
	{
		return m_data[stateOffset];
	}

	return -1; // Return an invalid value if the offset is out of bounds
}

PocketCameraConverter::IndexBuffer PocketCameraConverter::DecodeIndices(int index) const
{
	IndexBuffer out(kImageWidth * kImageHeight, 0);

	if (index < 0 || index >= kMaxImageCount)
		return out;
	if (m_data.size() < kExpectedFileSize)
		return out;

	uint32_t current = ImageOffset(index);
	constexpr int tilesX = kImageWidth / 8;  // 16
	constexpr int tilesY = kImageHeight / 8; // 14

	// Tile layout: 8x8 blocks. Within a block, 2 bytes per tile row;
	// after a block finishes, move to the block on the right; after a
	// row of blocks finishes, move down 8 lines.
	for (int ty = 0; ty < tilesY; ++ty)
	{
		for (int tx = 0; tx < tilesX; ++tx)
		{
			for (int row = 0; row < 8; ++row)
			{
				const uint8_t b1 = m_data[current];
				const uint8_t b2 = m_data[current + 1];
				current += 2;

				for (int bit = 0; bit < 8; ++bit)
				{
					const int shift = 7 - bit;
					const int v1 = (b1 >> shift) & 1; // white/silver bit-plane
					const int v2 = (b2 >> shift) & 1; // grey/black bit-plane
					const int colorIdx = v1 | (v2 << 1);

					const int x = tx * 8 + bit;
					const int y = ty * 8 + row;
					const size_t pixelIdx = (static_cast<size_t>(y) * kImageWidth + x);

					out[pixelIdx] = colorIdx;
				}
			}
		}
	}

	return out;
}