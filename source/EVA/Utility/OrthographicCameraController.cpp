#include "OrthographicCameraController.hpp"

#include "EVA/Core/Input.hpp"
#include "EVA/Core/Platform.hpp"
#include "EVA/Core/Timestep.hpp"

namespace EVA
{
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation) : m_AspectRatio(aspectRatio), m_Rotation(rotation),
		m_Camera(-m_AspectRatio* m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
	{

	}

	void OrthographicCameraController::OnUpdate()
	{
		auto dt = Platform::GetDeltaTime();

		if (Input::IsKeyPressed(KeyCode::W))
			m_CameraPosition.y += m_CameraTranslationSpeed * m_ZoomLevel * dt;
		if (Input::IsKeyPressed(KeyCode::S))
			m_CameraPosition.y -= m_CameraTranslationSpeed * m_ZoomLevel * dt;

		if (Input::IsKeyPressed(KeyCode::A))
			m_CameraPosition.x -= m_CameraTranslationSpeed * m_ZoomLevel * dt;
		if (Input::IsKeyPressed(KeyCode::D))
			m_CameraPosition.x += m_CameraTranslationSpeed * m_ZoomLevel * dt;

		m_Camera.SetPosition(m_CameraPosition);

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(KeyCode::Q))
				m_CameraRotation += m_CameraRotationSpeed * dt;
			if (Input::IsKeyPressed(KeyCode::E))
				m_CameraRotation -= m_CameraRotationSpeed * dt;

			m_Camera.SetRotation(m_CameraRotation);
		}
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel = glm::max(0.1f, m_ZoomLevel - e.GetYOffset() * m_CameraZoomSpeed);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}
}
