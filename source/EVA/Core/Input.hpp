#pragma once

#include "EVA/Core/KeyCodes.hpp"

namespace EVA
{
    class Input
    {
      public:
        enum class CursorMode
        {
            Normal,
            Hidden,
            Disabled
        };

        static bool IsKeyPressed(KeyCode key);

        static bool IsMouseButtonPressed(MouseCode button);
        static glm::vec2 GetMousePosition();
        static inline float GetMouseX() { return GetMousePosition().x; }
        static inline float GetMouseY() { return GetMousePosition().y; }
        static void SetCursorMode(const CursorMode mode);
        static CursorMode GetCursorMode() { return s_CursorMode; } 

      private:
        inline static CursorMode s_CursorMode = CursorMode::Normal;
    };
} // namespace EVA
