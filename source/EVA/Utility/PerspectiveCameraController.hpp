#pragma once

#include "EVA/Core/Input.hpp"
#include "EVA/Core/Platform.hpp"
#include "EVA/Renderer/PerspectiveCamera.hpp"
#include "Transform.hpp"

#include <imgui.h>

namespace EVA
{
    class PerspectiveCameraController
    {
        float m_AspectRatio;
        float m_Fov;
        float m_FovMin;
        float m_FovMax;
        float m_NearPlane;
        float m_FarPlane;

        float m_Pitch;
        float m_Yaw;

        PerspectiveCamera m_Camera;
        Transform m_Transform;

        glm::vec2 m_MousePos;

        float m_MovementSpeed = 1.0f;
        float m_MouseSensitivity = 0.3f; 
        float m_CameraZoomSpeed = 1;

      public:
        PerspectiveCameraController(glm::vec3 position, float pitch, float yaw, float aspect, float fov = 60, float fovMin = 10, float fovMax = 90, float nearPlane = 0.1, float farPlane = 1000) :
          m_AspectRatio(aspect),
          m_Fov(fov),
          m_FovMin(fovMin),
          m_FovMax(fovMax),
          m_NearPlane(nearPlane),
          m_FarPlane(farPlane),
          m_Pitch(pitch),
          m_Yaw(yaw),
          m_Camera(fov, aspect, nearPlane, farPlane)
        {
            m_Transform.SetPosition(position);
            m_MousePos = Input::GetMousePosition();
            OnUpdate();
        }

        void OnUpdate()
        {
            // Movement
            glm::vec3 movement = glm::vec3(0.0);

            // Front
            if (Input::IsKeyPressed(KeyCode::W)) movement += m_Transform.GetForward();

            // Back
            if (Input::IsKeyPressed(KeyCode::S)) movement -= m_Transform.GetForward();

            // Right
            if (Input::IsKeyPressed(KeyCode::D)) movement += m_Transform.GetRight();

            // Left
            if (Input::IsKeyPressed(KeyCode::A)) movement -= m_Transform.GetRight();

            // Up
            if (Input::IsKeyPressed(KeyCode::Space)) movement += m_Transform.GetUp();

            // Down
            if (Input::IsKeyPressed(KeyCode::LeftShift)) movement -= m_Transform.GetUp();

            m_Transform.Translate(movement * Platform::GetDeltaTime().GetSeconds() * m_MovementSpeed);

            // Mouse
            auto mousePos      = Input::GetMousePosition();
            auto mouseMovement = (mousePos - m_MousePos);
            mouseMovement      = glm::clamp(mouseMovement, -50.0f, 50.0f) * m_MouseSensitivity;
            m_MousePos         = mousePos;

            m_Pitch += mouseMovement.y * m_MouseSensitivity;
            m_Yaw += mouseMovement.x * m_MouseSensitivity;

            // Clamp
            m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
            if (m_Yaw < 0.0f)
                m_Yaw += 360.0f;
            else if (m_Yaw > 360.0f)
                m_Yaw -= 360.0f;

            m_Transform.SetOrientation(YAXIS, -m_Yaw);
            m_Transform.Rotate(-m_Transform.GetRight(), m_Pitch);

            m_Camera.SetView(m_Transform.GetPosition(), m_Transform.GetForward(), m_Transform.GetUp());
        }

        void OnResize(float width, float height) 
        {
            m_AspectRatio = width / height;
            m_Camera.SetProjection(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
        }

        void OnEvent(Event& e) 
        {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(PerspectiveCameraController::OnMouseScrolled));
            dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
        }

        bool OnWindowResized(WindowResizeEvent& e)
        {
            OnResize((float)e.GetWidth(), (float)e.GetHeight());
            return false;
        }

        bool OnMouseScrolled(MouseScrolledEvent& e)
        {
            m_Fov = glm::clamp(m_Fov - e.GetYOffset() * m_CameraZoomSpeed, m_FovMin, m_FovMax);
            m_Camera.SetProjection(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
            return false;
        }

        void Inspector()
        {
            ImGui::Text("Camera");

            bool changed = false;
            changed |= ImGui::SliderFloat("FOV", &m_Fov, m_FovMin, m_FovMax);
            changed |= ImGui::SliderFloat("Near", &m_NearPlane, 0, m_FarPlane);
            changed |= ImGui::SliderFloat("Far", &m_FarPlane, m_NearPlane, 10000);
            if (changed) { m_Camera.SetProjection(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane); }

            ImGui::SliderFloat("Movement speed", &m_MovementSpeed, 0, 10);
            ImGui::SliderFloat("Mouse sensitivity", &m_MouseSensitivity, 0, 1);
            ImGui::SliderFloat("Zoom sensitivity", &m_CameraZoomSpeed, 0, 5);

            auto pos = m_Transform.GetPosition();
            if (ImGui::InputFloat3("Position", glm::value_ptr(pos))) { m_Transform.SetPosition(pos); }
        }
        
        const Transform& GetTransform() const { return m_Transform; }
        float GetFov() const { return m_Fov; }
        float GetNearPlane() const { return m_NearPlane; }
        float GetFarPlane() const { return m_FarPlane; }

        PerspectiveCamera& GetCamera() { return m_Camera; }
        const PerspectiveCamera& GetCamera() const { return m_Camera; }
    };
} // namespace EVA
