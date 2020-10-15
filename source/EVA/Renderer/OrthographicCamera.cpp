#include "OrthographicCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace EVA 
{
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f))
	{
		RecalculateViewMatrix();
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::RecalculateViewMatrix()
	{
		m_ViewMatrix = glm::rotate(glm::mat4(1.0), glm::radians(-m_Rotation), glm::vec3(0, 0, 1));
		m_ViewMatrix = glm::translate(m_ViewMatrix, -m_Position);

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}
