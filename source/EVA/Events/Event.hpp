#pragma once

namespace EVA
{
	struct Event
	{
		enum class Type
		{
			None,
			WindowOpen, WindowClose, WindowFocus, WindowLostFocus, WindowResize, WindowMoved,
			KeyPressed, KeyReleased,
			MouseButtonPressed, MouseButtonReleased, MouseScrolled, MouseMoved
		};

		bool handled = false;

		[[nodiscard]] virtual Type GetType() const = 0;
        [[nodiscard]] virtual const char* GetName() const = 0;
        [[nodiscard]] virtual std::string ToString() const { return GetName(); }
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}

#define IMPL_EVENT(type) \
	static Event::Type GetStaticType() { return Event::Type::type; }\
	virtual Event::Type GetType() const override { return GetStaticType(); }\
	virtual const char* GetName() const override { return #type; }

	class EventDispatcher
	{
		Event& m_Event;

	public:
		explicit EventDispatcher(Event& event) : m_Event(event)
		{

		}

		template<typename T, typename F>
		bool Dispatch(const F& func)
		{
			if (m_Event.GetType() == T::GetStaticType())
			{
				m_Event.handled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	};
}
