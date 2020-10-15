#pragma once

#include <glm/glm.hpp>

#include "OrthographicCamera.hpp"
#include <EVA/Core/Timestep.hpp>
#include <EVA/Events/Window.hpp>
#include <EVA/Events/Mouse.hpp>

namespace EVA
{
	class OrthographicCameraController
	{
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		bool m_Rotation;

		glm::vec3 m_CameraPosition = glm::vec3(0.0f);
		float m_CameraRotation = 0.0f;

		float m_CameraTranslationSpeed = 2.0f;
		float m_CameraRotationSpeed = 90.0f;
		float m_CameraZoomSpeed = 0.2f;

		OrthographicCamera m_Camera;

	public:
		OrthographicCameraController(float aspectRatio, bool rotation = true);

		void OnUpdate();
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	};
}
