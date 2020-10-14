#pragma once

#include <glm/glm.hpp>

#include "KeyCodes.hpp"

namespace EVA {

	class Input
	{
	public:
		[[nodiscard]] static bool IsKeyPressed(KeyCode key);

		[[nodiscard]] static bool IsMouseButtonPressed(MouseCode button);
		[[nodiscard]] static glm::vec2 GetMousePosition();
		[[nodiscard]] static float GetMouseX();
		[[nodiscard]] static float GetMouseY();
	};
}
