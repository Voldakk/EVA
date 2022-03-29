#pragma once

#include "Base.hpp"

namespace EVA
{
    namespace TextureNodes
    {
        class Distance : public TextureNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Distance);

          public:
            Distance()
            {
                m_Texture      = TextureManager::CreateTexture(TextureSize, TextureSize, TextureR);
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

                glm::ivec2 dims          = glm::ivec2(inTexture->GetWidth(), inTexture->GetHeight());
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
    } // namespace TextureNodes
} // namespace EVA