#pragma once

#include <imgui.h>

namespace EVA
{
    struct Light
    {
        enum class Type
        {
            Directional,
            Point
        };

        Type type = Type::Directional;
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;

        // Directional
        glm::vec3 direction;

        // Point
        glm::vec3 position;
        float attenuation = 1.0f;

        void Inspector() 
        {
            ImGui::PushID(this);

            ImGui::Text("Light");
            ImGui::ColorEdit3("Color", glm::value_ptr(color));
            ImGui::SliderFloat("Intensity", &intensity, 0, 10.0f);
            
            bool isPoint = type == Type::Point;
            ImGui::Checkbox("Is point", &isPoint);
            type = isPoint ? Type::Point : Type::Directional;

            if (type == Light::Type::Directional)
            { 
                ImGui::InputFloat3("Direction", glm::value_ptr(direction));
            }
            else if (type == Light::Type::Point)
            {
                ImGui::InputFloat3("Position", glm::value_ptr(position));
                ImGui::SliderFloat("Attenuation", &attenuation, 0, 10);
            }

            ImGui::PopID();
        }

        void SetUniforms(const Ref<Shader>& shader, const std::string& uniformName, uint32_t index) 
        {
            shader->SetUniformFloat3(uniformName + "color", color * intensity);

            if (type == Light::Type::Directional)
            { 
                shader->SetUniformFloat4(uniformName + "position", glm::vec4(direction, 0.0f));
            }
            else if (type == Light::Type::Point)
            {
                shader->SetUniformFloat4(uniformName + "position", glm::vec4(position, 1.0f));
                shader->SetUniformFloat(uniformName + "attenuation", attenuation);
            }
        }
    };
}
