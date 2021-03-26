#pragma once

#include "Event.hpp"

namespace EVA
{
    class WindowOpenEvent : public Event
    {
      public:
        WindowOpenEvent() = default;

        IMPL_EVENT(WindowOpen, Category::Application)
    };

    class WindowCloseEvent : public Event
    {
      public:
        WindowCloseEvent() = default;

        IMPL_EVENT(WindowClose, Category::Application)
    };

    class WindowFocusEvent : public Event
    {
      public:
        WindowFocusEvent() = default;

        IMPL_EVENT(WindowFocus, Category::Application)
    };

    class WindowLostFocusEvent : public Event
    {
      public:
        WindowLostFocusEvent() = default;

        IMPL_EVENT(WindowLostFocus, Category::Application)
    };

    class WindowResizeEvent : public Event
    {
        int m_Width, m_Height;

      public:
        WindowResizeEvent(int width, int height) : m_Width(width), m_Height(height) {}

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
            return ss.str();
        }

        IMPL_EVENT(WindowResize, Category::Application)
    };

    class WindowMovedEvent : public Event
    {
      public:
        WindowMovedEvent() = default;

        IMPL_EVENT(WindowMoved, Category::Application)
    };
} // namespace EVA
