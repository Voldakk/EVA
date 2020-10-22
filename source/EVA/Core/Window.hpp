#pragma once

#include "EVA/Events/Event.hpp"

namespace EVA
{
    struct WindowProperties
    {
        std::string title;
        int width;
        int height;

        explicit WindowProperties(const std::string& title = "EVA Engine", int width = 1200, int height = 600) : title (title), width(width), height(height) {}
    };

    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        [[nodiscard]] virtual int GetWidth() const = 0;
        [[nodiscard]] virtual int GetHeight() const = 0;

        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
        virtual void SetVSync(bool enabled) = 0;
        [[nodiscard]] virtual bool IsVSync() const = 0;

        [[nodiscard]] virtual void* GetNativeWindow() const = 0;

        static std::unique_ptr<Window> Create(const WindowProperties& properties = WindowProperties());
    };
}
