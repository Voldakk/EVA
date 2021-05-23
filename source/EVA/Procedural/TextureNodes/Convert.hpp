#pragma once

#include "Base.hpp"

namespace EVA
{
    namespace TextureNodes
    {
        class HeightToNormal : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::HeightToNormal);

          public:
            HeightToNormal()
            {
                SetShader("height_to_normal.glsl");
                SetTexture(TextureFormat::RGBA32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Height to normal";
                AddOutputs<Ref<Texture>, 4>({{"Out", &m_Texture}});
                AddInputs<Ref<Texture>, 1>({{"In"}});
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat("u_Strength", m_Strength); }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                data.Serialize("Strength", m_Strength);

                processed &= !data.changed;
            }

          private:
            float m_Strength = 1;
        };

        class HeightToAmbientOcclusion : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::HeightToAmbientOcclusion);

          public:
            HeightToAmbientOcclusion()
            {
                SetShader("height_to_ao.glsl");
                SetTexture(TextureFormat::RGBA32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Height to AO";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}});
                AddInputs<Ref<Texture>, 1>({{"In"}});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat("u_HeightScale", m_HeightScale);
                m_Shader->SetUniformFloat("u_NormalStrength", m_NormalStrength);
                m_Shader->SetUniformFloat("u_Radius", m_Radius);
                m_Shader->SetUniformFloat("u_Bias", m_Bias);

                std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
                std::default_random_engine generator;

                for (unsigned int i = 0; i < 64; ++i)
                {
                    glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
                    sample = glm::normalize(sample);
                    sample *= randomFloats(generator);
                    float scale = float(i) / 64.0;

                    // scale samples s.t. they're more aligned to center of kernel
                    scale = glm::mix(0.1f, 1.0f, scale * scale);
                    sample *= scale;

                    m_Shader->SetUniformFloat3("u_Samples[" + std::to_string(i) + "]", sample);
                }

                for (unsigned int i = 0; i < 16; i++)
                {
                    glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)

                    m_Shader->SetUniformFloat3("u_Noise[" + std::to_string(i) + "]", noise);
                }
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector())
                {
                    data.changed |= ImGui::SliderFloat("Height scale", &m_HeightScale, 0, 1);
                    data.changed |= ImGui::SliderFloat("Radius", &m_Radius, 0, 1);
                    data.changed |= ImGui::SliderFloat("Bias", &m_Bias, 0, 0.2);
                }
                else
                {
                    data.Serialize("HeightScale", m_HeightScale);
                    data.Serialize("Radius", m_Radius);
                    data.Serialize("Bias", m_Bias);
                }

                data.Serialize("Normal Strength", m_NormalStrength);


                processed &= !data.changed;
            }

          private:
            float m_HeightScale    = 0.1f;
            float m_NormalStrength = 1.0f;

            float m_Radius = 0.5f;
            float m_Bias   = 0.025f;
        };

        class HeightToCurvature : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::HeightToCurvature);

          public:
            HeightToCurvature()
            {
                SetShader("height_to_curvature.glsl");
                SetTexture(TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Height to curvature";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}});
                AddInputs<Ref<Texture>, 1>({{"In"}});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformInt("u_Steps", m_Steps);
                m_Shader->SetUniformInt("u_StepSize", m_StepSize);
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector())
                {
                    data.changed |= ImGui::SliderInt("Steps", &m_Steps, 1, 10);
                    data.changed |= ImGui::SliderInt("StepSize", &m_StepSize, 1, 10);
                }
                else
                {
                    data.Serialize("Steps", m_Steps);
                    data.Serialize("StepSize", m_StepSize);
                }

                processed &= !data.changed;
            }

          private:
            int m_Steps    = 4;
            int m_StepSize = 2;
        };
    } // namespace TextureNodes
} // namespace EVA