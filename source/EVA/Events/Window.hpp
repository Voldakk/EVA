#pragma once
#include <sstream>

#include "Event.hpp"

namespace EVA
{
	class WindowOpenEvent : public Event
	{
	public:
		WindowOpenEvent() = default;

		IMPL_EVENT(WindowOpen)
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		IMPL_EVENT(WindowClose)
	};

	class WindowFocusEvent : public Event
	{
	public:
		WindowFocusEvent() = default;

		IMPL_EVENT(WindowFocus)
	};

	class WindowLostFocusEvent : public Event
	{
	public:
		WindowLostFocusEvent() = default;

		IMPL_EVENT(WindowLostFocus)
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

		IMPL_EVENT(WindowResize)
	};

	class WindowMovedEvent : public Event
	{
	public:
		WindowMovedEvent() = default;

		IMPL_EVENT(WindowMoved)
	};
}