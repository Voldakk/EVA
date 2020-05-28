#pragma once

#include "Event.hpp"

namespace EVA
{
	class MouseButtonEvent : public Event
	{
	protected:
		int m_Button;

		MouseButtonEvent(int button) : m_Button(button) {}
	
	public:
		int GetMouseButton() const { return m_Button; }	
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

		std::string ToString() const override
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
		MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

		std::string ToString() const override
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
		MouseScrolledEvent(float xOffset, float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override
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
		MouseMovedEvent(float x, float y) : m_MouseX(x), m_MouseY(y) {}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		IMPL_EVENT(MouseMoved)
	};
}
