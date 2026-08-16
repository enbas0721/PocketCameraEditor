// PocketCameraConverter.cpp
#include "PocketCameraConverter.h"

#include <fstream>

namespace
{
    // GB Camera's 4-shade palette (white/silver/grey/black).
    // Maps to the colorIdx (0-3) built from the two bit-plane bytes.
    // This is neutral grayscale.
    constexpr uint8_t kPalette[4][3] = {
        {255, 255, 255}, // 0: white
        {173, 173, 173}, // 1: silver
        {85,  85,  85 }, // 2: grey
        {0,   0,   0  }, // 3: black
    };
}

bool PocketCameraConverter::LoadFromFile(const std::string& path)
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
            // In the state vector, 0xFF indicates an inactive slot.
            return m_data[stateOffset] != 0xFF;
        }
    }
    return false;
}

PocketCameraConverter::RgbaBuffer PocketCameraConverter::DecodeImage(int index) const
{
    RgbaBuffer out(static_cast<size_t>(kImageWidth) * kImageHeight * 4, 0);

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
                    const size_t pixelIdx = (static_cast<size_t>(y) * kImageWidth + x) * 4;

                    out[pixelIdx + 0] = kPalette[colorIdx][0];
                    out[pixelIdx + 1] = kPalette[colorIdx][1];
                    out[pixelIdx + 2] = kPalette[colorIdx][2];
                    out[pixelIdx + 3] = 255;
                }
            }
        }
    }

    return out;
}
