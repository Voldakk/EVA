#pragma once

#include "Base.hpp"

namespace EVA
{
    namespace TextureNodes
    {
        class Uniform : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Uniform);

          public:
            Uniform()
            {
                // SetShader("uniform_rgba.glsl");
                // SetShader("uniform_grayscale.glsl");
                // SetTexture(TextureFormat::RGBA32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Uniform color";
                AddOutput<Ref<Texture>, 4>({"Out", &m_Texture});
            }

            void Process() override
            {
                if (m_Mode == 0)
                {
                    SetShader("uniform_grayscale.glsl");
                    SetTexture(TextureFormat::R32F);
                    SetOutputType<Ref<Texture>, 1>(0);
                }
                else
                {
                    SetShader("uniform_rgba.glsl");
                    SetTexture(TextureFormat::RGBA32F);
                    SetOutputType<Ref<Texture>, 4>(0);
                }

                ComputeNode::Process();
            }

            void SetUniforms() const override
            {
                if (m_Mode == 0)
                    m_Shader->SetUniformFloat("u_Value", m_Value);
                else
                    m_Shader->SetUniformFloat4("u_Color", m_Color);
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector())
                {
                    const char* items[] = {"Grayscale", "Color"};
                    data.changed |= ImGui::Combo("Mode", &m_Mode, items, IM_ARRAYSIZE(items));

                    if (m_Mode == 0)
                        data.changed |= ImGui::SliderFloat("Value", &m_Value, 0.0f, 1.0f);
                    else
                        data.changed |= ImGui::ColorPicker4("Color", glm::value_ptr(m_Color));
                }
                else
                {
                    data.Serialize("mode", m_Mode);
                    data.Serialize("value", m_Value);
                    data.Serialize("color", m_Color);
                }

                processed &= !data.changed;
            }

          private:
            int m_Mode    = 0;
            float m_Value = 0;

            glm::vec4 m_Color = glm::vec4(0, 0, 0, 1);
        };

        class Bricks : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Bricks);

          public:
            Bricks()
            {
                SetShader("brick.glsl");
                SetTexture(TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Bricks";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformInt2("u_NumBricks", m_numBricks);
                m_Shader->SetUniformFloat("u_Offset", m_Offset);
                m_Shader->SetUniformFloat2("u_Gap", m_Gap * 0.001f);
                m_Shader->SetUniformFloat2("u_Bevel", m_Bevel * 0.01f);
                m_Shader->SetUniformFloat2("u_Height", m_Height);
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                data.Serialize("Num bricks", m_numBricks);
                data.Serialize("Gap", m_Gap);
                data.Serialize("Bevel", m_Bevel);
                data.Serialize("Height", m_Height);

                if (data.Inspector())
                    data.changed |= ImGui::SliderFloat("Offset", &m_Offset, 0.0f, 1.0f);
                else
                    data.Serialize("Offset", m_Offset);

                processed &= !data.changed;
            }

          private:
            glm::ivec2 m_numBricks = {4, 8};
            float m_Offset         = 0.5f;
            glm::vec2 m_Gap        = {5.0f, 5.0f};
            glm::vec2 m_Bevel      = {1.0f, 1.0f};
            glm::vec2 m_Height     = {0.7f, 1.0f};
        };

    } // namespace TextureNodes
} // namespace EVA