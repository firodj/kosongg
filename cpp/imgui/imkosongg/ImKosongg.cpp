#include "ImKosongg.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include "kosongg/HsCppMacros.hpp"


ImKosongg::ImKosongg()
{
#ifdef _USE_HSCPP_
    auto cb = [this](hscpp::SwapInfo& info) {
		//info.Save("showDemoWindow",   m_showDemoWindow);

    };

    Hscpp_SetSwapHandler(cb);

	if (Hscpp_IsSwapping()) {
		return;
	}
#endif
	Creating();
}

ImKosongg::~ImKosongg()
{
#ifdef _USE_HSCPP_
    if (Hscpp_IsSwapping())
    {
        return;
    }
#endif
	Destroying();
}

void ImKosongg::Creating()
{

}
void ImKosongg::Destroying()
{

}

void ImKosongg::ShadeVertsLinearColorGradientSetAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{

}
ImVec2 ImKosongg::CalcButtonSizeWithText(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f)
{

}
bool ImKosongg::ColoredButtonV1(const char* label, const ImVec2& size_arg = ImVec2{0,0}, ImGuiButtonFlags flags = ImGuiButtonFlags_None
{

}

