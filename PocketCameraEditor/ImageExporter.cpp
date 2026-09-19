#include "ImageExporter.h"

#pragma warning(push)
#pragma warning(disable: 4996)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_WINDOWS_UTF8
#include <stb_image_write.h>
#pragma warning(pop)

ExportImage UpscaleNearest(std::span<const uint8_t> src, int srcWidth, int srcHeight, int scale)
{
	ExportImage result;
	if (scale < 1)
		return result;
	if (src.size() != static_cast<size_t>(srcWidth) * srcHeight * 4)
		return result;
	if (srcWidth <= 0 || srcHeight <= 0)
		return result;

	int dstWidth = srcWidth * scale;
	int dstHeight = srcHeight * scale;
	std::vector<uint8_t> dst(static_cast<size_t>(dstWidth) * dstHeight * 4);

	for (int dstY = 0; dstY < dstHeight; ++dstY)
	{
		for (int dstX = 0; dstX < dstWidth; ++dstX)
		{
			int srcX = dstX / scale;
			int srcY = dstY / scale;
			const size_t srcIdx = (static_cast<size_t>(srcY) * srcWidth + srcX) * 4;
			const size_t dstIdx = (static_cast<size_t>(dstY) * dstWidth + dstX) * 4;

			dst[dstIdx]		= src[srcIdx];
			dst[dstIdx + 1]	= src[srcIdx + 1];
			dst[dstIdx + 2]	= src[srcIdx + 2];
			dst[dstIdx + 3]	= src[srcIdx + 3];
		}
	}

	result.pixels = std::move(dst);
	result.width = dstWidth;
	result.height = dstHeight;

	return result;
}

bool SavePng(char const *filename, const ExportImage& image)
{
	if (image.pixels.size() != static_cast<size_t>(image.width) * image.height * 4)
		return false;

	// comp = 4, stride_bytes = 0 (default, auto)
	return stbi_write_png(filename, image.width, image.height, 4, image.pixels.data(), 0) != 0;
}