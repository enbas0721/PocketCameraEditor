#include "Palette.h"

std::vector<uint8_t> ApplyPalette(std::span<const uint8_t> indices, const Palette& palette)
{
	std::vector<uint8_t> out(indices.size() * 4);

	uint8_t paletteColors[4][3];
	for (int i = 0; i < 4; ++i)
	{
		paletteColors[i][0] = static_cast<uint8_t>(palette.colors[i][0] * 255.0f + 0.5f);
		paletteColors[i][1] = static_cast<uint8_t>(palette.colors[i][1] * 255.0f + 0.5f);
		paletteColors[i][2] = static_cast<uint8_t>(palette.colors[i][2] * 255.0f + 0.5f);
	}

	for (size_t i = 0; i < indices.size(); ++i)
	{
		const size_t currentIdx = i * 4;
		int colorIdx = indices[i];
		if (colorIdx > 3)
		{
			colorIdx = 0;
		}
		out[currentIdx + 0] = paletteColors[colorIdx][0];
		out[currentIdx + 1] = paletteColors[colorIdx][1];
		out[currentIdx + 2] = paletteColors[colorIdx][2];
		out[currentIdx + 3] = 255;
	}

	return out;
}