#pragma once

#include "Camera.hpp"

namespace EVA
{
    class PerspectiveCamera : public Camera
    {
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ViewProjectionMatrix;

      public:
        PerspectiveCamera(float fov, float aspect, float nearPlane, float farPlane) : m_ViewMatrix(glm::identity<glm::mat4>())
        {
            SetProjection(fov, aspect, nearPlane, farPlane);
        }

        void SetProjection(float fov, float aspect, float nearPlane, float farPlane)
        {
            m_ProjectionMatrix     = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
            m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
        }

        void SetView(glm::vec3 position, glm::vec3 forward, glm::vec3 up)
        {
            m_ViewMatrix           = glm::lookAt(position, position + forward, up);
            m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
        }

        const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() const override { return m_ViewProjectionMatrix; }
    };
} // namespace EVA
