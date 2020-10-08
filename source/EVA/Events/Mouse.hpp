#pragma once

#include "Event.hpp"

namespace EVA
{
	class MouseButtonEvent : public Event
	{
	protected:
		int m_Button;

        explicit MouseButtonEvent(int button) : m_Button(button) {}
	
	public:
		[[nodiscard]] int GetMouseButton() const { return m_Button; }
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		explicit MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

		[[nodiscard]] std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button;
			return ss.str();
		}

		IMPL_EVENT(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
        explicit MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

		[[nodiscard]] std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button;
			return ss.str();
		}

		IMPL_EVENT(MouseButtonReleased)
	};

	class MouseScrolledEvent : public Event
	{
		float m_XOffset, m_YOffset;

	public:
        explicit MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}

		[[nodiscard]] float GetXOffset() const { return m_XOffset; }
		[[nodiscard]] float GetYOffset() const { return m_YOffset; }

		[[nodiscard]] std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
			return ss.str();
		}

		IMPL_EVENT(MouseScrolled)
	};

	class MouseMovedEvent : public Event
	{
		float m_MouseX, m_MouseY;

	public:
        explicit MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

        [[nodiscard]] float GetX() const { return m_MouseX; }
        [[nodiscard]] float GetY() const { return m_MouseY; }

		[[nodiscard]] std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		IMPL_EVENT(MouseMoved)
	};
}
