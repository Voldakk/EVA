#pragma once

#include <deque>

namespace EVA
{
	template<typename T>
	class SlidingWindow
	{
		std::deque<T> m_Values;

	public:
		SlidingWindow(size_t count, T defaultValue = 0)
		{
			m_Values.resize(count, defaultValue);
		}

		inline void Add(T value)
		{
			m_Values.pop_front();
			m_Values.push_back(value);
		}

		T GetMin()
		{
			T min = 0;
			for (auto v : m_Values)
			{
				min = v < min ? v : min;
			}

			return min;
		}

		T GetMax()
		{
			T max = 0;
			for (auto v : m_Values)
			{
				max = v > max ? v : max;
			}

			return max;
		}

		T GetAverage()
		{
			T combined = 0;
			for (auto v : m_Values)
			{
				combined += v;
			}

			return combined / m_Values.size();
		}
	};
}
