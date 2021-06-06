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
                SetTexture(TextureRGBA);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Height to normal";
                AddOutput<Ref<Texture>, 4>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
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
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Height to AO";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
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
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Height to curvature";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
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

        class Distance : public TextureNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Distance);

          public:
            Distance()
            {
                m_Texture     = TextureManager::CreateTexture(TextureSize, TextureSize, TextureR);
                m_DataTexture1 = TextureManager::CreateTexture(TextureSize, TextureSize, TextureFormat::RG32UI);
                m_DataTexture2 = TextureManager::CreateTexture(TextureSize, TextureSize, TextureFormat::RG32UI);

                m_SeedShader    = AssetManager::Load<Shader>(std::string(ShaderPath) + "distance/seed.glsl");
                m_StepShader    = AssetManager::Load<Shader>(std::string(ShaderPath) + "distance/step.glsl");
                m_TextureShader = AssetManager::Load<Shader>(std::string(ShaderPath) + "distance/texture.glsl");
            }

            void SetupNode() override
            {
                TextureNode::SetupNode();
                name = "Distance";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
            }

            bool ProcessTextureNode() override
            {
                EVA_PROFILE_FUNCTION();
                const Ref<Texture>* ref = GetInputDataPtr<Ref<Texture>>(0);

                if (ref != nullptr && *ref)
                {
                    GenerateTexture(*ref);
                    return true;
                }
                return false;
            }

            void Serialize(DataObject& data) override
            {
                TextureNode::Serialize(data);

                if (data.Inspector())
                {
                    data.changed |= ImGui::SliderFloat("Threshold", &m_Threshold, 0, 1);
                    data.changed |= ImGui::SliderFloat("Max distance", &m_MaxDistance, 0, 1.5f);
                }
                else
                {
                    data.Serialize("Threshold", m_Threshold);
                    data.Serialize("Max distance", m_MaxDistance);
                }

                data.Serialize("Blend", m_Blend);

                processed &= !data.changed;
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

          private:
            float m_Threshold   = 0.0f;
            float m_MaxDistance = 0.0f;
            bool m_Blend        = false;

            Ref<Texture> m_DataTexture1, m_DataTexture2;
            Ref<Texture> m_Texture;
            Ref<Shader> m_SeedShader, m_StepShader, m_TextureShader;

            void GenerateTexture(const Ref<Texture>& inTexture) 
            { 
                constexpr glm::ivec2 workGroupSize = glm::ivec2(16);

                glm::ivec2 dims = glm::ivec2(inTexture->GetWidth(), inTexture->GetHeight());
                glm::ivec2 numWorkGroups = (dims + workGroupSize - 1) / workGroupSize;

                Ref<Texture>* currentDataTexture = &m_DataTexture1;
                Ref<Texture>* prevDataTexture    = &m_DataTexture2;
                {
                    EVA_PROFILE_SCOPE("Seed");
                    m_SeedShader->Bind();

                    m_SeedShader->SetUniformFloat("u_Threshold", m_Threshold);
                    m_SeedShader->BindImageTexture(0, inTexture, Access::ReadOnly);
                    m_SeedShader->BindImageTexture(1, *currentDataTexture, Access::WriteOnly);

                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                    m_SeedShader->DispatchCompute(numWorkGroups.x, numWorkGroups.y, 1, workGroupSize.x, workGroupSize.y, 1);
                }

                int stepSize = dims.x;
                {
                    EVA_PROFILE_SCOPE("Step");
                    m_StepShader->Bind();

                    while (stepSize > 1)
                    {
                        std::swap(currentDataTexture, prevDataTexture);
                        stepSize /= 2;
                        m_StepShader->SetUniformInt("u_StepSize", stepSize);
                        m_StepShader->SetUniformFloat("u_MaxDistance", m_MaxDistance);

                        m_StepShader->BindImageTexture(0, *prevDataTexture, Access::ReadOnly);
                        m_StepShader->BindImageTexture(1, *currentDataTexture, Access::WriteOnly);

                        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                        m_StepShader->DispatchCompute(numWorkGroups.x, numWorkGroups.y, 1, workGroupSize.x, workGroupSize.y, 1);
                    }
                } 

                {
                    EVA_PROFILE_SCOPE("Generate texture");
                    m_TextureShader->Bind();

                    m_TextureShader->SetUniformBool("u_Blend", m_Blend);
                    m_TextureShader->SetUniformFloat("u_MaxDistance", m_MaxDistance);

                    m_TextureShader->BindImageTexture(0, inTexture, Access::ReadOnly);
                    m_TextureShader->BindImageTexture(1, *currentDataTexture, Access::ReadOnly);
                    m_TextureShader->BindImageTexture(2, m_Texture, Access::WriteOnly);

                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                    m_TextureShader->DispatchCompute(numWorkGroups.x, numWorkGroups.y, 1, workGroupSize.x, workGroupSize.y, 1);
                }
            }
        };

        class EdgeDetect : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::EdgeDetect);

          public:
            EdgeDetect()
            {
                SetShader("edge_detect.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Edge detect";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat("u_Threshold", m_Threshold);
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector()) { data.changed |= ImGui::SliderFloat("Threshold", &m_Threshold, 0, 1); }
                else
                {
                    data.Serialize("Threshold", m_Threshold);
                }

                processed &= !data.changed;
            }

          private:
            float m_Threshold = 0.1f;
        };
    } // namespace TextureNodes
} // namespace EVA