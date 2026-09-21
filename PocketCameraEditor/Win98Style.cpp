// Win98Style.cpp
#include "Win98Style.h"

#include "imgui.h"

namespace
{
	// The Windows 98 system palette, as 0-255 components.
	constexpr ImU32 kFace        = IM_COL32(192, 192, 192, 255); // 3D face: window and button background
	constexpr ImU32 kHighlight   = IM_COL32(255, 255, 255, 255); // 3D highlight: top/left bevel edge
	constexpr ImU32 kShadow      = IM_COL32(128, 128, 128, 255); // 3D shadow: bottom/right bevel edge
	constexpr ImU32 kDarkShadow  = IM_COL32( 64,  64,  64, 255); // 3D dark shadow: outer bevel edge
	constexpr ImU32 kWindow      = IM_COL32(255, 255, 255, 255); // Editable field background
	constexpr ImU32 kText        = IM_COL32(  0,   0,   0, 255);
	constexpr ImU32 kGrayText    = IM_COL32(128, 128, 128, 255);
	constexpr ImU32 kActiveTitle = IM_COL32(  0,   0, 128, 255); // The classic navy title bar
	constexpr ImU32 kInactTitle  = IM_COL32(128, 128, 128, 255);
	constexpr ImU32 kTitleText   = IM_COL32(255, 255, 255, 255);
	constexpr ImU32 kScrollTrack = IM_COL32(223, 223, 223, 255);

	// ImGui stores colors as ImVec4, so every constant above needs converting.
	ImVec4 ToVec4(ImU32 packed)
	{
		return ImGui::ColorConvertU32ToFloat4(packed);
	}

	void Set(ImGuiCol index, ImU32 packed)
	{
		ImGui::GetStyle().Colors[index] = ToVec4(packed);
	}
}

void ApplyWin98Style()
{
	ImGuiStyle& style = ImGui::GetStyle();

	// ---- Metrics -------------------------------------------------------
	// Windows 98 has no rounded corners anywhere.
	style.WindowRounding    = 0.0f;
	style.ChildRounding     = 0.0f;
	style.FrameRounding     = 0.0f;
	style.PopupRounding     = 0.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabRounding      = 0.0f;
	style.TabRounding       = 0.0f;

	// A single-pixel outline stands in for the real two-tone bevel.
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize  = 1.0f;
	style.FrameBorderSize  = 1.0f;
	style.PopupBorderSize  = 1.0f;

	style.WindowPadding   = ImVec2(6.0f, 6.0f);
	style.FramePadding    = ImVec2(4.0f, 3.0f);
	style.ItemSpacing     = ImVec2(6.0f, 4.0f);
	style.ItemInnerSpacing= ImVec2(4.0f, 4.0f);

	// Chunky scrollbars, like the real thing.
	style.ScrollbarSize = 16.0f;
	style.GrabMinSize   = 16.0f;

	// Title text sits on the left in Windows 98.
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);

	// ---- Colors --------------------------------------------------------
	Set(ImGuiCol_Text,                 kText);
	Set(ImGuiCol_TextDisabled,         kGrayText);

	Set(ImGuiCol_WindowBg,             kFace);
	Set(ImGuiCol_ChildBg,              kFace);
	Set(ImGuiCol_PopupBg,              kFace);

	Set(ImGuiCol_Border,               kDarkShadow);
	Set(ImGuiCol_BorderShadow,         IM_COL32(0, 0, 0, 0));

	// Editable fields are white and sunken.
	Set(ImGuiCol_FrameBg,              kWindow);
	Set(ImGuiCol_FrameBgHovered,       kWindow);
	Set(ImGuiCol_FrameBgActive,        kWindow);

	Set(ImGuiCol_TitleBg,              kInactTitle);
	Set(ImGuiCol_TitleBgActive,        kActiveTitle);
	Set(ImGuiCol_TitleBgCollapsed,     kInactTitle);
	Set(ImGuiCol_MenuBarBg,            kFace);

	Set(ImGuiCol_ScrollbarBg,          kScrollTrack);
	Set(ImGuiCol_ScrollbarGrab,        kFace);
	Set(ImGuiCol_ScrollbarGrabHovered, kFace);
	Set(ImGuiCol_ScrollbarGrabActive,  kShadow);

	Set(ImGuiCol_CheckMark,            kText);
	Set(ImGuiCol_CheckboxSelectedBg,   kWindow);

	Set(ImGuiCol_SliderGrab,           kFace);
	Set(ImGuiCol_SliderGrabActive,     kShadow);

	// Buttons keep the same face color in every state; on real hardware the
	// bevel flips instead of the fill changing.
	Set(ImGuiCol_Button,               kFace);
	Set(ImGuiCol_ButtonHovered,        IM_COL32(208, 208, 208, 255));
	Set(ImGuiCol_ButtonActive,         kShadow);

	// [NOTE] The authentic selection color is the navy kActiveTitle, but ImGui
	// draws item text with a single Text color, so black on navy would be hard
	// to read. A mid gray keeps the list legible.
	Set(ImGuiCol_Header,               IM_COL32(160, 160, 160, 255));
	Set(ImGuiCol_HeaderHovered,        IM_COL32(176, 176, 176, 255));
	Set(ImGuiCol_HeaderActive,         IM_COL32(160, 160, 160, 255));

	Set(ImGuiCol_Separator,            kShadow);
	Set(ImGuiCol_SeparatorHovered,     kShadow);
	Set(ImGuiCol_SeparatorActive,      kDarkShadow);

	Set(ImGuiCol_ResizeGrip,           kFace);
	Set(ImGuiCol_ResizeGripHovered,    kHighlight);
	Set(ImGuiCol_ResizeGripActive,     kShadow);

	Set(ImGuiCol_InputTextCursor,      kText);

	Set(ImGuiCol_Tab,                  kFace);
	Set(ImGuiCol_TabHovered,           IM_COL32(208, 208, 208, 255));
	Set(ImGuiCol_TabSelected,          kWindow);
	Set(ImGuiCol_TabDimmed,            kFace);
	Set(ImGuiCol_TabDimmedSelected,    kFace);

	Set(ImGuiCol_PlotLines,            kText);
	Set(ImGuiCol_PlotLinesHovered,     kActiveTitle);
	Set(ImGuiCol_PlotHistogram,        kActiveTitle);
	Set(ImGuiCol_PlotHistogramHovered, kText);

	Set(ImGuiCol_TableHeaderBg,        kFace);
	Set(ImGuiCol_TableBorderStrong,    kDarkShadow);
	Set(ImGuiCol_TableBorderLight,     kShadow);
	Set(ImGuiCol_TableRowBg,           IM_COL32(0, 0, 0, 0));
	Set(ImGuiCol_TableRowBgAlt,        IM_COL32(0, 0, 0, 16));

	// Text selection inside input fields: white on navy, as in the real UI.
	Set(ImGuiCol_TextSelectedBg,       kActiveTitle);
	Set(ImGuiCol_TextLink,             kActiveTitle);

	Set(ImGuiCol_NavCursor,            kDarkShadow);
	Set(ImGuiCol_DragDropTarget,       kActiveTitle);
	Set(ImGuiCol_ModalWindowDimBg,     IM_COL32(0, 0, 0, 64));
}
