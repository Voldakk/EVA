#pragma once

#include <glm/glm.hpp>

#include "EVA/Core/KeyCodes.hpp"

namespace EVA {

	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static inline float GetMouseX() { return GetMousePosition().x; }
		static inline float GetMouseY() { return GetMousePosition().y; }
	};
}
