#pragma once
#include <vector>
#include <span>

struct ExportImage
{
	std::vector<uint8_t> pixels;
	int width = 0;
	int height = 0;
};

// Upscale the image using nearest neighbor interpolation.
ExportImage UpscaleNearest(std::span<const uint8_t> src, int srcWidth, int srcHeight, int scale);

// Save the image as a PNG file.
bool SavePng(char const *filename, const ExportImage& image);