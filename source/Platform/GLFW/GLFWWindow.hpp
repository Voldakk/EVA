#pragma once

#include <GLFW/glfw3.h>

#include "EVA/Core/Window.hpp"
#include "EVA/Renderer/GraphicsContext.hpp"

namespace EVA
{
    class GLFWWindow : public Window
    {
      public:
        explicit GLFWWindow(const WindowProperties& properties);
        ~GLFWWindow() override;

        void OnUpdate() override;

        int GetWidth() const override { return m_Data.width; }
        int GetHeight() const override { return m_Data.height; }

        void SetEventCallback(const EventCallbackFn& callback) override { m_Data.eventCallback = callback; }
        void SetVSync(bool enabled) override;
        bool IsVSync() const override { return m_Data.vSync; };

        virtual void* GetNativeWindow() const override { return m_Window; }

      private:
        virtual void Init(const WindowProperties& properties);
        virtual void Shutdown();

        struct WindowData
        {
            std::string title;
            int width, height;
            bool vSync;
            EventCallbackFn eventCallback;
        };
        WindowData m_Data;

        GLFWwindow* m_Window;
        GraphicsContext* m_Context;
    };
} // namespace EVA
