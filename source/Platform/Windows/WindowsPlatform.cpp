#include "EVA/Core/Platform.hpp"

#include <GLFW/glfw3.h>

namespace EVA
{
	float Platform::GetTime()
	{
		return (float)glfwGetTime();
	}
}
