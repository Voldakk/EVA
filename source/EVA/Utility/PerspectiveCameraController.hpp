#pragma once

#include "EVA/Core/Input.hpp"
#include "EVA/Core/Platform.hpp"
#include "EVA/Renderer/PerspectiveCamera.hpp"

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

        glm::vec3 m_Position;
        glm::quat m_Orientation;

        glm::vec3 m_Forward;
        glm::vec3 m_Right;
        glm::vec3 m_Up;

        glm::vec2 m_MousePos;

        float m_MouseSensitivity = 0.3f; 
        float m_CameraZoomSpeed = 1;

        void CalculateAxis()
        {
            m_Forward = glm::normalize(m_Orientation * glm::vec3(0, 0, 1));
            m_Right   = glm::normalize(m_Orientation * glm::vec3(1, 0, 0));
            m_Up      = glm::normalize(m_Orientation * glm::vec3(0, 1, 0));
        }

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
          m_Camera(fov, aspect, nearPlane, farPlane),
          m_Position(position)
        {
            m_MousePos = Input::GetMousePosition();
            OnUpdate();
        }

        void OnUpdate()
        {
            // Movement
            glm::vec3 movement = glm::vec3(0.0);

            // Front
            if (Input::IsKeyPressed(KeyCode::W)) movement += m_Forward;

            // Back
            if (Input::IsKeyPressed(KeyCode::S)) movement -= m_Forward;

            // Right
            if (Input::IsKeyPressed(KeyCode::D)) movement -= m_Right;

            // Left
            if (Input::IsKeyPressed(KeyCode::A)) movement += m_Right;

            // Up
            if (Input::IsKeyPressed(KeyCode::Space)) movement += m_Up;

            // Down
            if (Input::IsKeyPressed(KeyCode::LeftShift)) movement -= m_Up;

            m_Position += movement * Platform::GetDeltaTime().GetSeconds();

            // Mouse
            auto mousePos      = Input::GetMousePosition();
            auto mouseMovement = (mousePos - m_MousePos) * m_MouseSensitivity;
            m_MousePos         = mousePos;

            m_Pitch += mouseMovement.y * m_MouseSensitivity;
            m_Yaw += mouseMovement.x * m_MouseSensitivity;

            // Clamp
            m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
            if (m_Yaw < 0.0f)
                m_Yaw += 360.0f;
            else if (m_Yaw > 360.0f)
                m_Yaw -= 360.0f;

            m_Orientation = glm::angleAxis(glm::radians(-m_Yaw), glm::vec3(0, 1, 0));
            CalculateAxis();
            m_Orientation = glm::angleAxis(glm::radians(m_Pitch), m_Right) * m_Orientation;
            CalculateAxis();

            m_Camera.SetView(m_Position, m_Forward, m_Up);
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
            ImGui::SliderFloat("FOV", &m_Fov, m_FovMin, m_FovMax);
            ImGui::SliderFloat("Near", &m_NearPlane, 0, m_FarPlane);
            ImGui::SliderFloat("Far", &m_FarPlane, m_NearPlane, 10000);
            ImGui::SliderFloat("Mouse sensitivity", &m_MouseSensitivity, 0, 1);
            ImGui::SliderFloat("Zoom sensitivity", &m_CameraZoomSpeed, 0, 1);
            ImGui::InputFloat3("Position", glm::value_ptr(m_Position));
        }

        glm::vec3 GetPosition() const { return m_Position; }
        glm::vec3 GetForward() const { return m_Forward; }
        glm::vec3 GetRight() const { return m_Right; }
        glm::vec3 GetUp() const { return m_Up; }
        float GetFov() const { return m_Fov; }

        PerspectiveCamera& GetCamera() { return m_Camera; }
        const PerspectiveCamera& GetCamera() const { return m_Camera; }
    };
} // namespace EVA
