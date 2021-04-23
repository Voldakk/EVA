#include "EVA/Core/Application.hpp"
#include "EVA/Core/Input.hpp"

#include <GLFW/glfw3.h>

namespace EVA
{
    bool Input::IsKeyPressed(const KeyCode key)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state   = glfwGetKey(window, static_cast<int32_t>(key));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(const MouseCode button)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state   = glfwGetMouseButton(window, static_cast<int32_t>(button));
        return state == GLFW_PRESS;
    }

    glm::vec2 Input::GetMousePosition()
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        return {(float)xpos, (float)ypos};
    }

    void Input::SetCursorMode(const CursorMode mode)
    {
        s_CursorMode = mode;
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        switch (mode)
        {
            case CursorMode::Normal: glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); return;
            case CursorMode::Hidden: glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); return;
            case CursorMode::Disabled: glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); return;
        }
        throw;
    }
} // namespace EVA
