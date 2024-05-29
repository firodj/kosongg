#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui.h"
#include "imgui_internal.h"
#include "kosongg/Component.h"

// TODO: shadow https://github.com/ocornut/imgui/issues/1329

// https://github.com/ocornut/imgui/issues/3710
void ImGui::ShadeVertsLinearColorGradientSetAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    const int col0_r = (int)(col0 >> IM_COL32_R_SHIFT) & 0xFF;
    const int col0_g = (int)(col0 >> IM_COL32_G_SHIFT) & 0xFF;
    const int col0_b = (int)(col0 >> IM_COL32_B_SHIFT) & 0xFF;
    const int col0_a = (int)(col0 >> IM_COL32_A_SHIFT) & 0xFF;
    const int col_delta_r = ((int)(col1 >> IM_COL32_R_SHIFT) & 0xFF) - col0_r;
    const int col_delta_g = ((int)(col1 >> IM_COL32_G_SHIFT) & 0xFF) - col0_g;
    const int col_delta_b = ((int)(col1 >> IM_COL32_B_SHIFT) & 0xFF) - col0_b;
    const int col_delta_a = ((int)(col1 >> IM_COL32_A_SHIFT) & 0xFF) - col0_a;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = (int)(col0_r + col_delta_r * t);
        int g = (int)(col0_g + col_delta_g * t);
        int b = (int)(col0_b + col_delta_b * t);
        int a = (int)(col0_a + col_delta_a * t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (a << IM_COL32_A_SHIFT);
    }
}

ImVec2 ImGui::CalcButtonSizeWithText(const char* text, const char* text_end, bool hide_text_after_double_hash, float wrap_width) {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    ImVec2 label_size = ImGui::CalcTextSize(text, text_end, hide_text_after_double_hash);
    label_size.x += style.FramePadding.x * 2.0f;
    label_size.y += style.FramePadding.y * 2.0f;
    return label_size;
}

#define COLORED_BUTTON_MODE 3

// https://github.com/ocornut/imgui/issues/4722
bool ImGui::ColoredButtonV1(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    //if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
    //    flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    ImU32 bg_color_1 = IM_COL32(0xff, 0xff, 0xff, 255*0.47);
    ImU32 bg_color_2 = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

#if COLORED_BUTTON_MODE != 3
    ImVec4 bg1f = ColorConvertU32ToFloat4(bg_color_1);
    ImVec4 bg2f = ColorConvertU32ToFloat4(bg_color_2);
    ImColor blend = ImColor();

    // https://stackoverflow.com/questions/726549/algorithm-for-additive-color-mixing-for-rgb-values
    blend.Value.w = 1 - (1 - bg1f.w) * (1 - bg2f.w);
    if (blend.Value.w >= 1.0e-6) {  // not-Fully transparent -- R,G,B important
        blend.Value.x = bg1f.x * bg1f.w / blend.Value.w + bg2f.x * bg2f.w * (1 - bg1f.w) / blend.Value.w;
        blend.Value.y = bg1f.y * bg1f.w / blend.Value.w + bg2f.y * bg2f.w * (1 - bg1f.w) / blend.Value.w;
        blend.Value.z = bg1f.z * bg1f.w / blend.Value.w + bg2f.z * bg2f.w * (1 - bg1f.w) / blend.Value.w;
    }

    bg_color_1 = (ImU32)blend;
#endif

    const bool is_gradient = bg_color_1 != bg_color_2;
#if 0
    if (held || hovered)
    {
        // Modify colors (ultimately this can be prebaked in the style)
        float h_increase = (held && hovered) ? 0.02f : 0.02f;
        float v_increase = (held && hovered) ? 0.20f : 0.07f;

        ImVec4 bg1f = ColorConvertU32ToFloat4(bg_color_1);
        ColorConvertRGBtoHSV(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg1f.x = ImMin(bg1f.x + h_increase, 1.0f);
        bg1f.z = ImMin(bg1f.z + v_increase, 1.0f);
        ColorConvertHSVtoRGB(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg_color_1 = GetColorU32(bg1f);
        if (is_gradient)
        {
            ImVec4 bg2f = ColorConvertU32ToFloat4(bg_color_2);
            ColorConvertRGBtoHSV(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg2f.z = ImMin(bg2f.z + h_increase, 1.0f);
            bg2f.z = ImMin(bg2f.z + v_increase, 1.0f);
            ColorConvertHSVtoRGB(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg_color_2 = GetColorU32(bg2f);
        }
        else
        {
            bg_color_2 = bg_color_1;
        }
    }
#endif

    ImGui::RenderNavHighlight(bb, id);

    //
#if COLORED_BUTTON_MODE == 1
    // V1 : faster but prevents rounding
    window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max, bg_color_1, bg_color_1, bg_color_2, bg_color_2);
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), 0.0f, 0, g.Style.FrameBorderSize);
#elif COLORED_BUTTON_MODE == 2
    // V2
    int vert_start_idx = window->DrawList->VtxBuffer.Size;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, IM_COL32(0, 0, 0, 0xff), g.Style.FrameRounding);
    int vert_end_idx = window->DrawList->VtxBuffer.Size;
    if (is_gradient)
        ShadeVertsLinearColorGradientKeepAlpha(window->DrawList, vert_start_idx, vert_end_idx, bb.Min, bb.GetBL(), bg_color_1, bg_color_2);
#elif COLORED_BUTTON_MODE == 3
    // V3
    ImGui::RenderFrame(bb.Min, bb.Max, bg_color_2, true, style.FrameRounding);

    if (is_gradient) {
        int vert_start_idx = window->DrawList->VtxBuffer.Size;
        window->DrawList->AddRectFilled(bb.Min, bb.Max, IM_COL32(0, 0, 0, 0xff), g.Style.FrameRounding);
        int vert_end_idx = window->DrawList->VtxBuffer.Size;
        ImU32 bg_color_3 = IM_COL32(0xff, 0xff, 0xff, 255*0);
        ShadeVertsLinearColorGradientSetAlpha(window->DrawList, vert_start_idx, vert_end_idx, bb.Min, bb.GetBL(), bg_color_1, bg_color_3);
    }
#endif
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), g.Style.FrameRounding, 0, g.Style.FrameBorderSize);

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");
    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    //IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}
