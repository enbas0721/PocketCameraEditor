// AppState.h
// Everything the UI reads and writes between frames.
#pragma once

#include <string>

#include "PhotoLibrary.h"

struct AppState
{
	// The opened .sav and its 30 slots.
	PhotoLibrary library;

	// Slot shown in the Viewer window. Not to be confused with Slot::isSelected,
	// which marks slots for batch export.
	int currentIndex = 0;

	// Magnification applied when writing a PNG. The Viewer itself is fixed at 4x.
	int exportScale = 4;

	// Hides slots that the camera considers erased.
	bool hideDeleted = false;

	// Result of the last load/save, shown until the next one replaces it.
	std::string statusMessage;

	// Path of the currently opened .sav, for display only.
	std::string currentFilePath = "No file loaded.";
};
