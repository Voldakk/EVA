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

		enum Category
		{
			None = 0,
			Application = BIT(0),
			Input = BIT(1),
			Keyboard = BIT(2),
			Mouse = BIT(3),
			MouseButton = BIT(4)
		};

		bool handled = false;

		virtual Type GetType() const = 0;
        virtual const char* GetName() const = 0;

		virtual int GetCategoryFlags() const = 0;
		bool IsInCategory(Category category) { return GetCategoryFlags() & category; }

        virtual std::string ToString() const { return GetName(); }
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}

#define IMPL_EVENT(type, category) \
	static Event::Type GetStaticType() { return Event::Type::type; }\
	virtual Event::Type GetType() const override { return GetStaticType(); }\
	virtual const char* GetName() const override { return #type; }\
	virtual int GetCategoryFlags() const override { return (category); }

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
