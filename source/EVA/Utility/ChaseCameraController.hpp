#pragma once

#include "EVA/Renderer/PerspectiveCamera.hpp"
#include "Transform.hpp"

namespace EVA
{
    class ChaseCameraController
    {
      public:
        PerspectiveCamera& camera;
        Transform& self;
        Transform& target;
        float scalar                 = 7.0f;
        glm::vec3 offset             = glm::vec3(0.0f, 0.2f, -1.5f);
        glm::vec2 offsetScaleRange   = glm::vec2(1.0f, 3.0f);
        float offsetScale            = 1.5f;
        float offsetScaleSensitivity = 0.1f;

        ChaseCameraController(PerspectiveCamera& camera, Transform& self, Transform& target) :
          camera(camera), self(self), target(target) {};

        void OnUpdate()
        {
            self.SetOrientation(target.GetOrientation());

            const auto targetPos = target.GetPosition() + target.LocalToWorld(offset * offsetScale);
            self.SetPosition(glm::mix(self.GetPosition(), targetPos, Platform::GetDeltaTime().GetSeconds() * scalar));

            camera.SetView(self.GetPosition(), self.GetForward(), self.GetUp());
        }

        void OnEvent(Event& e)
        {
            EVA_PROFILE_FUNCTION();
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(ChaseCameraController::OnMouseScrolled));
        }

        bool OnMouseScrolled(MouseScrolledEvent& e)
        {
            EVA_PROFILE_FUNCTION();
            offsetScale = glm::clamp(offsetScale - e.GetYOffset() * offsetScaleSensitivity, offsetScaleRange.x, offsetScaleRange.y);
            return false;
        }

        void Inspector()
        {
            ImGui::PushID(this);

            ImGui::Text("Chase camara");
            ImGui::SliderFloat("Scalar", &scalar, 0, 50.0f);
            ImGui::InputFloat3("Offset", glm::value_ptr(offset));
            ImGui::InputFloat2("Offset scale min/max", glm::value_ptr(offsetScaleRange));
            ImGui::SliderFloat("Offset scale sensitivity", &offsetScaleSensitivity, 0, 10.0f);

            auto pos = self.GetPosition();
            if (ImGui::InputFloat3("Position", glm::value_ptr(pos))) { self.SetPosition(pos); }

            ImGui::PopID();
        }
    };
} // namespace EVA