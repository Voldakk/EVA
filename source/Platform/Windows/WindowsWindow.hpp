#pragma once

#include <GLFW/glfw3.h>

#include "EVA/Core/Window.hpp"
#include "EVA/Renderer/GraphicsContext.hpp"

namespace EVA
{
    class WindowsWindow : public Window
    {
        struct WindowData
        {
            std::string title;
            int width, height;
            bool vSync;
            EventCallbackFn  eventCallback;
        };

        GLFWwindow* m_Window;
        WindowData m_Data;

        GraphicsContext* m_Context;

    public:

        explicit WindowsWindow(const WindowProperties& properties);
        ~WindowsWindow() override;

        void OnUpdate() override;

        [[nodiscard]] int GetWidth() const override {return m_Data.width;}
        [[nodiscard]] int GetHeight() const override {return m_Data.height;}

        void SetEventCallback(const EventCallbackFn &callback) override { m_Data.eventCallback = callback; }
        void SetVSync(bool enabled) override;
        [[nodiscard]] bool IsVSync() const override { return m_Data.vSync; };

        virtual void* GetNativeWindow() const { return m_Window; }

    private:

        virtual void Init(const WindowProperties& properties);
        virtual void Shutdown();
    };
}
