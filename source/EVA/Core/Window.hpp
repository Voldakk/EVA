#pragma once

#include "EVA/Events/Event.hpp"

namespace EVA
{
    struct WindowProperties
    {
        std::string title;
        int width;
        int height;

        explicit WindowProperties(const std::string& title = "EVA Engine", int width = 1920, int height = 1080) :
          title(title), width(width), height(height)
        {
        }
    };

    class Window
    {
      public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Window() = default;

        virtual void OnUpdate() = 0;

        virtual int GetWidth() const  = 0;
        virtual int GetHeight() const = 0;
        float GetAspect() const { return static_cast<float>(GetWidth()) / static_cast<float>(GetHeight()); };

        virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
        virtual void SetVSync(bool enabled)                            = 0;
        virtual bool IsVSync() const                                   = 0;

        virtual void* GetNativeWindow() const = 0;

        static std::unique_ptr<Window> Create(const WindowProperties& properties = WindowProperties());
    };
} // namespace EVA
