#pragma once

#ifdef _USE_HSCPP_
#include "hscpp/module/Tracker.h"
#include "hscpp/mem/Ref.h"
#endif
#include "kosongg/hscpp_macros.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

namespace kosongg {
class ImControl {

	HSCPP_TRACK(ImControl, "ImControl");

public:
	hscpp_virtual ~ImControl();
	ImControl();

	void Creating();
	void Destroying();

	hscpp_virtual void ShadeVertsLinearColorGradientSetAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1);
	hscpp_virtual ImVec2 CalcButtonSizeWithText(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);
	hscpp_virtual bool ColoredButtonV1(const char* label, const ImVec2& size_arg = ImVec2{0,0}, ImGuiButtonFlags flags = ImGuiButtonFlags_None);
};
};
