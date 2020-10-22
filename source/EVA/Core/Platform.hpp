#pragma once

#include "EVA/Core/Timestep.hpp"

namespace EVA
{
	class Platform
	{
		inline static Timestep m_DeltaTime;

	public:

		inline static Timestep GetDeltaTime()
		{
			return m_DeltaTime;
		}

		inline static void SetDeltaTime(Timestep timestep)
		{
			m_DeltaTime = timestep;
		}

		// Platform dependent
		static float GetTime();
	};
}
