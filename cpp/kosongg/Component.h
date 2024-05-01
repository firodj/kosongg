#pragma once

#include "imgui.h"

// Header
namespace ImGui
{
  void ShadeVertsLinearColorGradientSetAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1);

  bool ColoredButtonV1(const char* label, const ImVec2& size_arg = ImVec2{0,0}, ImGuiButtonFlags flags = ImGuiButtonFlags_None);
}