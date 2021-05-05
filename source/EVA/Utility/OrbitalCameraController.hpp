#pragma once

#include "EVA/Core/Input.hpp"
#include "EVA/Core/Platform.hpp"
#include "EVA/Renderer/PerspectiveCamera.hpp"
#include "Transform.hpp"

#include <imgui.h>

namespace EVA
{
    class OrbitalCameraController
    {
        float m_AspectRatio;
        float m_Fov;
        float m_NearPlane;
        float m_FarPlane;

        float m_Pitch;
        float m_Yaw;
        float m_Distance;
        float m_MinDistance;
        float m_MaxDistance;

        PerspectiveCamera m_Camera;
        Transform m_CenterTransform;
        Transform m_CameraTransform;

        glm::vec2 m_MousePos;

        float m_MovementSpeed    = 1.0f;
        float m_MouseSensitivity = 0.3f;
        float m_CameraZoomSpeed  = 1;

      public:
        OrbitalCameraController(float aspect, glm::vec3 position, float pitch, float yaw, float distance = 2.0f, float minDistance = 1.0f, float maxDistance = 10.0f, 
                                float fov = 60, float nearPlane = 0.1, float farPlane = 1000) :
          m_AspectRatio(aspect),
          m_Fov(fov),
          m_NearPlane(nearPlane),
          m_FarPlane(farPlane),
          m_Pitch(pitch),
          m_Yaw(yaw),
          m_Distance(distance),
          m_MinDistance(minDistance),
          m_MaxDistance(maxDistance),
          m_Camera(fov, aspect, nearPlane, farPlane)
        {
            EVA_PROFILE_FUNCTION();

            m_CenterTransform.SetPosition(position);
            m_CameraTransform.SetParent(&m_CenterTransform);
            m_MousePos = Input::GetMousePosition();
            OnUpdate();
        }

        void OnUpdate()
        {
            EVA_PROFILE_FUNCTION();

            // Movement
            glm::vec3 movement = glm::vec3(0.0);

            // Front
            if (Input::IsKeyPressed(KeyCode::W)) movement += m_CenterTransform.GetForward();

            // Back
            if (Input::IsKeyPressed(KeyCode::S)) movement -= m_CenterTransform.GetForward();

            // Right
            if (Input::IsKeyPressed(KeyCode::D)) movement += m_CenterTransform.GetRight();

            // Left
            if (Input::IsKeyPressed(KeyCode::A)) movement -= m_CenterTransform.GetRight();

            // Up
            if (Input::IsKeyPressed(KeyCode::Space)) movement += YAXIS;

            // Down
            if (Input::IsKeyPressed(KeyCode::LeftShift)) movement -= YAXIS;

            m_CenterTransform.Translate(movement * Platform::GetDeltaTime().GetSeconds() * m_MovementSpeed);

            // Mouse
            auto mousePos      = Input::GetMousePosition();
            auto mouseMovement = (mousePos - m_MousePos);
            mouseMovement      = glm::clamp(mouseMovement, -50.0f, 50.0f) * m_MouseSensitivity;
            m_MousePos         = mousePos;

            m_Pitch += mouseMovement.y;
            m_Yaw += mouseMovement.x;

            // Clamp
            m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
            if (m_Yaw < 0.0f)
                m_Yaw += 360.0f;
            else if (m_Yaw > 360.0f)
                m_Yaw -= 360.0f;

            UpdateTransforms();
        }

        void UpdateTransforms() 
        {
            m_CenterTransform.SetOrientation(YAXIS, -m_Yaw);
            m_CenterTransform.Rotate(-m_CenterTransform.GetRight(), m_Pitch);

            m_CameraTransform.SetPosition(-ZAXIS * m_Distance);

            m_Camera.SetView(m_CameraTransform.GetPosition(), m_CameraTransform.GetForward(), m_CameraTransform.GetUp());
        }

        void OnResize(float width, float height)
        {
            EVA_PROFILE_FUNCTION();
            m_AspectRatio = width / height;
            m_Camera.SetProjection(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
            UpdateTransforms();
        }

        void OnEvent(Event& e)
        {
            EVA_PROFILE_FUNCTION();
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(OrbitalCameraController::OnMouseScrolled));
            dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OrbitalCameraController::OnWindowResized));
        }

        bool OnWindowResized(WindowResizeEvent& e)
        {
            OnResize((float)e.GetWidth(), (float)e.GetHeight());
            return false;
        }

        bool OnMouseScrolled(MouseScrolledEvent& e)
        {
            EVA_PROFILE_FUNCTION();
            m_Distance = glm::clamp(m_Distance - e.GetYOffset() * m_CameraZoomSpeed, m_MinDistance, m_MaxDistance);
            UpdateTransforms();
            return false;
        }

        void Inspector()
        {
            EVA_PROFILE_FUNCTION();
            ImGui::PushID(this);
            ImGui::Text("Camera");

            bool changed = false;
            changed |= ImGui::SliderFloat("FOV", &m_Fov, 30.0f, 90.0f);
            changed |= ImGui::SliderFloat("Near", &m_NearPlane, 0, m_FarPlane);
            changed |= ImGui::SliderFloat("Far", &m_FarPlane, m_NearPlane, 10000);
            if (changed) { m_Camera.SetProjection(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane); }
            ImGui::Spacing();
            ImGui::SliderFloat("Movement speed", &m_MovementSpeed, 0, 10);
            ImGui::SliderFloat("Mouse sensitivity", &m_MouseSensitivity, 0, 1);
            ImGui::SliderFloat("Zoom sensitivity", &m_CameraZoomSpeed, 0, 5);
            ImGui::Spacing();
            ImGui::SliderFloat("Distance", &m_Distance, m_MinDistance, m_MaxDistance);
            ImGui::SliderFloat("Min distance", &m_MinDistance, 0, m_MaxDistance);
            ImGui::SliderFloat("Max distance", &m_MaxDistance, m_MinDistance, 100.0f);
            ImGui::Spacing();
            auto pos = m_CenterTransform.GetPosition();
            if (ImGui::InputFloat3("Position", glm::value_ptr(pos))) { m_CenterTransform.SetPosition(pos); }
            ImGui::PopID();
        }

        Transform& GetTransform() { return m_CenterTransform; }
        const Transform& GetTransform() const { return m_CenterTransform; }
        float GetFov() const { return m_Fov; }
        float GetNearPlane() const { return m_NearPlane; }
        float GetFarPlane() const { return m_FarPlane; }

        PerspectiveCamera& GetCamera() { return m_Camera; }
        const PerspectiveCamera& GetCamera() const { return m_Camera; }
    };
} // namespace EVA
