#pragma once

#include "imgui.h"

namespace ImGui
{
    enum class CurveEditorFlags
    {
        NO_TANGENTS = 1 << 0,
        SHOW_GRID   = 1 << 1,
        RESET       = 1 << 2
    };

    int CurveEditor(const char* label, float* values, int points_count, const ImVec2& editor_size, ImU32 flags, int* new_count);
} // namespace ImGui