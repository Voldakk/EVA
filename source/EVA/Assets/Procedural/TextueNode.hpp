#pragma once

#include "EVA.hpp"
#include "NodeEditor.hpp"

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
                ImGui::Image(*reinterpret_cast<void**>(&textureId), ImVec2 {100, 100}, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});
            }
        };

        class PassthroughNode : public BaseNode
        {
          public:
            PassthroughNode()          = default;
            virtual ~PassthroughNode() = default;

            void Process() override
            {
                const Ref<Texture>* ref = GetInputDataPtr<Ref<Texture>>(0);
                m_Texture               = *ref;
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            void SetupNode() override
            {
                name = "Passthrough";
                AddInputs<Ref<Texture>>({{"In"}});
                AddOutputs<Ref<Texture>>({{"Out", &m_Texture}});
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

            void SetupNode() override 
            { 
                AddOutputs<Ref<Texture>>({{"Out"}}); 
            }

            void Process() override
            {
                m_Shader->Bind();
                m_Shader->ResetTextureUnit();
                m_Shader->BindImageTexture(0, m_Texture, TextureAccess::WriteOnly);

                for (size_t i = 0; i < inputs.size(); i++)
                {
                    if (!InputIsType<Ref<Texture>>(i)) continue;

                    const Ref<Texture>& ref = GetInputData<Ref<Texture>>(i);
                    m_Shader->BindImageTexture(i + 1, ref, TextureAccess::ReadOnly);
                }

                SetUniforms();

                uint32_t numWorkGroupsX = m_Texture->GetWidth() / m_WorkGroupSize;
                uint32_t numWorkGroupsY = m_Texture->GetHeight() / m_WorkGroupSize;
                m_Shader->DispatchCompute(numWorkGroupsX, numWorkGroupsY, 1, m_WorkGroupSize, m_WorkGroupSize, 1);
            }

            void DrawFields() override
            {
                BaseNode::DrawFields();

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

          protected:
            Ref<Texture> m_Texture;
            Ref<Shader> m_Shader;
            std::string m_ShaderName;
            uint32_t m_WorkGroupSize = 16;
        };

        class VoronoiNoiseNode : public ComputeNode
        {
          public:
            VoronoiNoiseNode()
            {
                SetShader("voronoi.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Voronoi noise";
                SetOutputData(0, &m_Texture);
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat("u_Scale", m_Scale); }

            void DrawFields() override
            {
                ComputeNode::DrawFields();

                bool changed = false;
                changed |= ImGui::InputFloat("Scale", &m_Scale);

                processed &= !changed;
            }

          private:
            float m_Scale = 10.0f;
        };

        class GradientNoiseNode : public ComputeNode
        {
          public:
            GradientNoiseNode()
            {
                SetShader("gradient_noise.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Gradient noise";
                SetOutputData(0, &m_Texture);
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat("u_Scale", m_Scale); }

            void DrawFields() override
            {
                ComputeNode::DrawFields();

                bool changed = false;
                changed |= ImGui::InputFloat("Scale", &m_Scale);

                processed &= !changed;
            }

          private:
            float m_Scale = 10.0f;
        };

        class BlendNode : public ComputeNode
        {
          public:
            BlendNode()
            {
                SetShader("blend.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Blend";
                SetOutputData(0, &m_Texture);

                AddInputs<Ref<Texture>>({{"A"}, {"B"}});
                AddInputs<float>({{"Frac", false}});
            }

            void Process() override
            {
                m_Frac = GetInputData(2, m_Frac);
                ComputeNode::Process();
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat("u_Frac", m_Frac); }

            void DrawFields() override
            {
                ComputeNode::DrawFields();

                bool changed = false;
                if (!InputConnected(2)) { changed |= ImGui::SliderFloat("Frac", &m_Frac, 0.0f, 1.0f); }

                processed &= !changed;
            }

          private:
            float m_Frac = 0.5f;
        };

        class BrickNode : public ComputeNode
        {
          public:
            BrickNode()
            {
                SetShader("brick.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Brick";
                SetOutputData(0, &m_Texture);
            }

            void SetUniforms() const override 
            { 
                m_Shader->SetUniformInt2("u_NumBricks", m_numBricks);
                m_Shader->SetUniformFloat("u_Offset", m_Offset);
                m_Shader->SetUniformFloat2("u_Gap", m_Gap * 0.001f);
                m_Shader->SetUniformFloat2("u_Bevel", m_Bevel * 0.01f);
                m_Shader->SetUniformFloat2("u_Height", m_Height); 
            }

            void DrawFields() override
            {
                ComputeNode::DrawFields();

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
    } // namespace TextureNodes
} // namespace EVA