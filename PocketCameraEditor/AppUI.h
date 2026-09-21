// AppUI.h
// Draws the application's ImGui windows.
#pragma once

struct AppState;

// Submits every window for the current frame.
// Must be called between ImGui::NewFrame() and ImGui::Render().
void DrawAppUI(AppState& app);
