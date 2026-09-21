#pragma once
#include <cstdint>
#include <vector>
#include <span>

struct Palette
{
	float colors[4][3]; // 4 colors, each with RGB components
};

struct PalettePreset
{
	const char* name;
	Palette palette;
};

constexpr PalettePreset kPresets[] = {
	{ "Pocket",   { { { 1.000f, 1.000f, 1.000f }, { 0.678f, 0.678f, 0.678f }, { 0.333f, 0.333f, 0.333f }, { 0.000f, 0.000f, 0.000f } } } },
	{ "GB",       { { { 0.608f, 0.737f, 0.059f }, { 0.545f, 0.675f, 0.059f }, { 0.188f, 0.384f, 0.188f }, { 0.059f, 0.220f, 0.059f } } } },
	{ "Light",    { { { 0.000f, 0.710f, 0.506f }, { 0.000f, 0.604f, 0.443f }, { 0.000f, 0.412f, 0.290f }, { 0.000f, 0.310f, 0.231f } } } },
	{ "Sepia",    { { { 1.000f, 0.945f, 0.855f }, { 0.800f, 0.690f, 0.525f }, { 0.545f, 0.416f, 0.267f }, { 0.235f, 0.157f, 0.098f } } } },
	{ "Inverted", { { { 0.000f, 0.000f, 0.000f }, { 0.333f, 0.333f, 0.333f }, { 0.678f, 0.678f, 0.678f }, { 1.000f, 1.000f, 1.000f } } } }
};

// Applies a 4-color palette to the decoded indices and returns an RGBA buffer.
std::vector<uint8_t> ApplyPalette(std::span<const uint8_t> indices, const Palette& palette);

