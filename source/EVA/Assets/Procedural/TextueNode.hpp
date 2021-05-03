#pragma once

#include "EVA.hpp"
#include "NodeEditor.hpp"
#include <chrono>

namespace EVA
{
    namespace TextureNodes
    {
        constexpr std::string_view ShaderPath = "./assets/procedural/shaders/";

        class BaseNode : public NE::Node
        {
          public:
            BaseNode()          = default;
            virtual ~BaseNode() = default;

            virtual Ref<Texture> GetTexture() const = 0;

            void DrawFields() override
            {
                auto texture       = GetTexture();
                uint32_t textureId = 0;
                if (texture != nullptr) textureId = texture->GetRendererId();
                ImGui::Image(*reinterpret_cast<void**>(&textureId), ImVec2 {150, 150}, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});
            }

            virtual void DrawProperties()
            {
                
            }

            virtual std::chrono::microseconds GetProcessTimeMs() const { return std::chrono::microseconds(0); };
        };

        class Passthrough : public BaseNode
        {
          public:
            Passthrough()              = default;
            virtual ~Passthrough() = default;

            void Process() override
            {
                const Ref<Texture>* ref = GetInputDataPtr<Ref<Texture>>(0);
                m_Texture               = *ref;
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            void SetupNode() override
            {
                name = "Passthrough";
                AddInputs<Ref<Texture>, 1>({{"In"}});
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}});
            }

          private:
            Ref<Texture> m_Texture;
        };

        class ComputeNode : public BaseNode
        {
          protected:
            ComputeNode() = default;

          public:
            virtual ~ComputeNode() = default;

            void Process() override
            {
                auto startTimepoint = std::chrono::steady_clock::now();

                m_Shader->Bind();
                m_Shader->ResetTextureUnit();
                m_Shader->BindImageTexture(0, m_Texture, TextureAccess::WriteOnly);

                for (size_t i = 0; i < inputs.size(); i++)
                {
                    if (!InputIsType<Ref<Texture>, 1>(i) && !InputIsType<Ref<Texture>, 4>(i)) continue;

                    const Ref<Texture>& ref = GetInputData<Ref<Texture>>(i);
                    m_Shader->BindImageTexture(i + 1, ref, TextureAccess::ReadOnly);
                }

                SetUniforms();

                uint32_t numWorkGroupsX = m_Texture->GetWidth() / m_WorkGroupSize;
                uint32_t numWorkGroupsY = m_Texture->GetHeight() / m_WorkGroupSize;
                m_Shader->DispatchCompute(numWorkGroupsX, numWorkGroupsY, 1, m_WorkGroupSize, m_WorkGroupSize, 1);

                auto endTimepoint = std::chrono::steady_clock::now();
                m_ProcessTime     = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() -
                                std::chrono::time_point_cast<std::chrono::microseconds>(startTimepoint).time_since_epoch();
            }

            void DrawFields() override
            {
                BaseNode::DrawFields();
                ImGui::Text("Time: %6.2f ms", m_ProcessTime.count() / 1000.0f);
            }

            void DrawProperties() override
            {
                BaseNode::DrawProperties();
                ImGui::Text("Time: %6.2f ms", m_ProcessTime.count() / 1000.0f);
                if (ImGui::Button("Reload shader")) 
                { 
                    m_Shader = Shader::Create(std::string(ShaderPath) + m_ShaderName); 
                    DoProcess();
                }
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            virtual void SetUniforms() const {};

            void SetShader(const std::string& name) 
            { 
                m_ShaderName = name;
                m_Shader = Shader::Create(std::string(ShaderPath) + name);
            }

            std::chrono::microseconds GetProcessTimeMs() const override { return m_ProcessTime; };

          protected:
            Ref<Texture> m_Texture;
            Ref<Shader> m_Shader;
            std::string m_ShaderName;
            uint32_t m_WorkGroupSize = 16;

            std::chrono::microseconds m_ProcessTime {};
        };

        class VoronoiNoise : public ComputeNode
        {
          public:
            VoronoiNoise()
            {
                SetShader("voronoi.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Voronoi noise";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}}); 
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat("u_Scale", m_Scale); }

            void DrawProperties() override
            {
                ComputeNode::DrawProperties();

                bool changed = false;
                changed |= ImGui::InputFloat("Scale", &m_Scale);

                processed &= !changed;
            }

          private:
            float m_Scale = 10.0f;
        };

        class GradientNoise : public ComputeNode
        {
          public:
            GradientNoise()
            {
                SetShader("gradient_noise.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Gradient noise";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}}); 
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat("u_Scale", m_Scale); }

            void DrawProperties() override
            {
                ComputeNode::DrawProperties();

                bool changed = false;
                changed |= ImGui::InputFloat("Scale", &m_Scale);

                processed &= !changed;
            }

          private:
            float m_Scale = 10.0f;
        };

        class BlendGrayscale : public ComputeNode
        {
          public:
            BlendGrayscale()
            {
                SetShader("blend_grayscale.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Blend Grayscale";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}}); 
                AddInputs<Ref<Texture>, 1>({{"A"}, {"B"}});
                AddInputs<float>({{"Frac", false}});
            }

            void Process() override
            {
                m_Opacity = GetInputData(2, m_Opacity);
                ComputeNode::Process();
            }

            void SetUniforms() const override 
            { 
                m_Shader->SetUniformFloat("u_Opacity", m_Opacity);
                m_Shader->SetUniformInt("u_BlendMode", m_BlendMode); 
            }

            void DrawProperties() override
            {
                ComputeNode::DrawProperties();

                bool changed = false;
                if (!InputConnected(2)) { changed |= ImGui::SliderFloat("Opacity", &m_Opacity, 0.0f, 1.0f); }

                const char* items[] = {"Copy", "Add", "Substract", "Multiply", "Divide", "Min", "Max"};
                changed |= ImGui::Combo("Blend mode", &m_BlendMode, items, IM_ARRAYSIZE(items));

                processed &= !changed;
            }

          private:
            float m_Opacity = 0.5f;
            int m_BlendMode = 0;
        };

        class Bricks : public ComputeNode
        {
          public:
            Bricks()
            {
                SetShader("brick.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Bricks";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}}); 
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformInt2("u_NumBricks", m_numBricks);
                m_Shader->SetUniformFloat("u_Offset", m_Offset);
                m_Shader->SetUniformFloat2("u_Gap", m_Gap * 0.001f);
                m_Shader->SetUniformFloat2("u_Bevel", m_Bevel * 0.01f);
                m_Shader->SetUniformFloat2("u_Height", m_Height);
            }

            void DrawProperties() override
            {
                ComputeNode::DrawProperties();

                bool changed = false;
                changed |= ImGui::InputInt2("Num bricks", glm::value_ptr(m_numBricks));

                changed |= ImGui::SliderFloat("Offset", &m_Offset, 0.0f, 1.0f);

                changed |= ImGui::InputFloat2("Gap", glm::value_ptr(m_Gap));
                changed |= ImGui::InputFloat2("Bevel", glm::value_ptr(m_Bevel));
                changed |= ImGui::InputFloat2("Height", glm::value_ptr(m_Height));

                processed &= !changed;
            }

          private:
            glm::ivec2 m_numBricks = {4, 8};
            float m_Offset         = 0.5f;
            glm::vec2 m_Gap        = {5.0f, 5.0f};
            glm::vec2 m_Bevel      = {1.0f, 1.0f};
            glm::vec2 m_Height     = {0.7f, 1.0f};
        };

        class LevelsGrayscale : public ComputeNode
        {
          public:
            LevelsGrayscale()
            {
                SetShader("levles_grayscale.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Levels Grayscale";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}});
                AddInputs<Ref<Texture>, 1>({{"In"}});

            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat2("u_InputRange", m_InputRange);
                m_Shader->SetUniformFloat2("u_OutputRange", m_OutputRange);
                m_Shader->SetUniformFloat("u_Midtone", m_Midtone);
            }

            void DrawProperties() override
            {
                ComputeNode::DrawProperties();

                bool changed = false;

                changed |= ImGui::SliderFloat2("Input range", glm::value_ptr(m_InputRange), 0.0f, 1.0f);
                changed |= ImGui::SliderFloat2("Output range", glm::value_ptr(m_OutputRange), 0.0f, 1.0f);
                changed |= ImGui::SliderFloat("Midtone", &m_Midtone, 0.0f, 1.0f);

                processed &= !changed;
            }

          private:
            glm::vec2 m_InputRange  = {0.0f, 1.0f};
            glm::vec2 m_OutputRange = {0.0f, 1.0f};
            float m_Midtone         = 0.5f;
        };
    } // namespace TextureNodes
} // namespace EVA