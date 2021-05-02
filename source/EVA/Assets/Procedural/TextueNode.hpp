#pragma once

#include "EVA.hpp"
#include "NodeEditor.hpp"

namespace EVA
{
    namespace TextureNodes
    {
        constexpr std::string_view ShaderPath = "./assets/procedural/shaders/";
        constexpr float TextureViewSize       = 100.0f;

        inline static const uint32_t PinTexture = NE::NodeEditor::GetPinType<Texture>();
        inline static const uint32_t PinFloat   = NE::NodeEditor::GetPinType<float>();


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
                ImGui::Image(*reinterpret_cast<void**>(&textureId), ImVec2 {TextureViewSize, TextureViewSize}, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});
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
                AddPins({{NE::PinKind::Input, PinTexture, "In"}, {NE::PinKind::Output, PinTexture, "Out"}});
                SetOutputData(0, &m_Texture);
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

            void SetupNode() override { AddPins({{NE::PinKind::Output, PinTexture, "Out"}}); }

            void Process() override
            {
                m_Shader->Bind();
                m_Shader->ResetTextureUnit();
                m_Shader->BindImageTexture(0, m_Texture, TextureAccess::WriteOnly);

                for (size_t i = 0; i < inputs.size(); i++)
                {
                    if (inputs[i].type != PinTexture) continue;

                    const Ref<Texture>& ref = GetInputData<Ref<Texture>>(i);
                    m_Shader->BindImageTexture(i + 1, ref, TextureAccess::ReadOnly);
                }

                SetUniforms();

                uint32_t numWorkGroupsX = m_Texture->GetWidth() / m_WorkGroupSize;
                uint32_t numWorkGroupsY = m_Texture->GetHeight() / m_WorkGroupSize;
                m_Shader->DispatchCompute(numWorkGroupsX, numWorkGroupsY, 1, m_WorkGroupSize, m_WorkGroupSize, 1);
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            virtual void SetUniforms() const {};

          protected:
            Ref<Texture> m_Texture;
            Ref<Shader> m_Shader;
            uint32_t m_WorkGroupSize = 16;
        };

        class VoronoiNoiseNode : public ComputeNode
        {
          public:
            VoronoiNoiseNode()
            {
                m_Shader  = Shader::Create(std::string(ShaderPath) + "voronoi.glsl");
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
                BaseNode::DrawFields();

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
                m_Shader  = Shader::Create(std::string(ShaderPath) + "gradient_noise.glsl");
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
                BaseNode::DrawFields();

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
                m_Shader  = Shader::Create(std::string(ShaderPath) + "blend.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Blend";
                SetOutputData(0, &m_Texture);

                AddPins({{NE::PinKind::Input, PinTexture, "A"}, {NE::PinKind::Input, PinTexture, "B"}, {NE::PinKind::Input, PinFloat, "Frac", false}});
            }

            void Process() override
            {
                m_Frac = GetInputData(2, m_Frac);
                ComputeNode::Process();
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat("u_Frac", m_Frac); }

            void DrawFields() override
            {
                BaseNode::DrawFields();

                bool changed = false;
                if (!InputConnected(2)) { changed |= ImGui::SliderFloat("Frac", &m_Frac, 0.0f, 1.0f); }

                processed &= !changed;
            }

          private:
            float m_Frac = 0.5f;
        };
    } // namespace TextureNodes
} // namespace EVA