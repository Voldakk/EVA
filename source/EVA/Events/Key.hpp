#pragma once

#include "Event.hpp"

namespace EVA
{
	class KeyEvent : public Event
	{
	protected:
		int m_KeyCode;

        explicit KeyEvent(int keycode) : m_KeyCode(keycode){}

	public:
        [[nodiscard]] int GetKeyCode() const { return m_KeyCode; }
	};

	class KeyPressedEvent : public KeyEvent
	{
		int m_RepeatCount;

	public:
		KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

        [[nodiscard]] int GetRepeatCount() const { return m_RepeatCount; }
		[[nodiscard]] bool Repeated() const { return m_RepeatCount > 0; }

        [[nodiscard]] std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
			return ss.str();
		}

		IMPL_EVENT(KeyPressed)
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
        explicit KeyReleasedEvent(int keycode) : KeyEvent(keycode){}

        [[nodiscard]] std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		IMPL_EVENT(KeyReleased)
	};
}
