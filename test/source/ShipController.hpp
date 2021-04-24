#pragma once

#include "EVA.hpp"
#include <imgui.h>

namespace EVA
{
    class ShipController
    {
      public:
        ShipController(glm::vec3 position) 
        {
            EVA_PROFILE_FUNCTION();

            m_Transform.SetPosition(position);
            m_MousePos = Input::GetMousePosition();
        }

        void OnUpdate() 
        {
            EVA_PROFILE_FUNCTION();

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
            mouseMovement      = glm::clamp(mouseMovement, -50.0f, 50.0f) * m_TurnSpeed;
            m_MousePos         = mousePos;

            float roll = 0;
            // Roll CCW
            if (Input::IsKeyPressed(KeyCode::Q)) roll -= m_RollSpeed;

            // Roll CW
            if (Input::IsKeyPressed(KeyCode::E)) roll += m_RollSpeed;

            roll *= Platform::GetDeltaTime().GetSeconds();

            m_Transform.Rotate(m_Transform.GetUp(), -mouseMovement.x);
            m_Transform.Rotate(m_Transform.GetRight(), -mouseMovement.y);
            m_Transform.Rotate(m_Transform.GetForward(), roll);
        }

        void Inspector() 
        {
            EVA_PROFILE_FUNCTION();

            ImGui::Text("Ship controller");

            ImGui::SliderFloat("Movement speed##ShipController", &m_MovementSpeed, 0, 10);
            ImGui::SliderFloat("Turn speed##ShipController", &m_TurnSpeed, 0, 10);
            ImGui::SliderFloat("Roll speed##ShipController", &m_RollSpeed, 0, 360);

            auto pos = m_Transform.GetPosition();
            if (ImGui::InputFloat3("Position##ShipController", glm::value_ptr(pos))) { m_Transform.SetPosition(pos); }
        }

        Transform& GetTransform() { return m_Transform; }
        const Transform& GetTransform() const { return m_Transform; }

      private:
        Transform m_Transform;
        float m_MovementSpeed = 2.0f;
        float m_TurnSpeed     = 0.05f;
        float m_RollSpeed     = 60.0f;

        glm::vec2 m_MousePos;
    };
}
