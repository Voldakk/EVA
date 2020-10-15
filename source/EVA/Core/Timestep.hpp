#pragma once

namespace EVA
{
	class Timestep
	{
		float m_Time;

	public:

		Timestep(float time = 0.0f) : m_Time(time) {}

		inline operator float() { return m_Time; }

		inline float GetSeconds() { return m_Time; }
		inline float GetMilliSeconds() { return m_Time * 1000.0f; }
	};
}
