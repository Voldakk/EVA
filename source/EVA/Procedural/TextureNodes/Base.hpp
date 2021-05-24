#pragma once

#include "EVA.hpp"
#include "../NodeEditor.hpp"
#include <chrono>

namespace EVA
{
    namespace TextureNodes
    {
        constexpr std::string_view ShaderPath = "procedural/shaders/";
        constexpr uint32_t TextureSize = 2048;

        class TextureNode : public NE::Node
        {
          protected:
            TextureNode() = default;

            Ref<Texture>& TextureWhite()
            {
                static Ref<Texture> texture;
                if (!texture) { texture = TextureUtilities::Uniform(1.0f, TextureFormat::R32F, TextureSize); }
                return texture;
            }

            Ref<Texture>& TextureBlack()
            {
                static Ref<Texture> texture;
                if (!texture) { texture = TextureUtilities::Uniform(0.0f, TextureFormat::R32F, TextureSize); }
                return texture;
            }

          public:
            virtual ~TextureNode() = default;

            virtual Ref<Texture> GetTexture() const = 0;

            void DrawFields() override
            {
                auto texture       = GetTexture();
                uint32_t textureId = 0;
                if (texture != nullptr) textureId = texture->GetRendererId();
                ImGui::Image(*reinterpret_cast<void**>(&textureId), ImVec2 {150, 150}, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});
            }

            virtual std::chrono::microseconds GetProcessTimeMs() const { return std::chrono::microseconds(0); };

            void Serialize(DataObject& data) override { NE::Node::Serialize(data); }
        };

        class Passthrough : public TextureNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Passthrough);

          public:
            Passthrough()          = default;
            virtual ~Passthrough() = default;

            void Process() override
            {
                const Ref<Texture>* ref = GetInputDataPtr<Ref<Texture>>(0);
                m_Texture               = *ref;

                if (m_Texture != nullptr)
                {
                    auto channels = GetTextureChannels(m_Texture->GetFormat());
                    if (channels == 1) SetOutputType<Ref<Texture>, 1>(0);
                    if (channels == 4) SetOutputType<Ref<Texture>, 4>(0);
                }
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            void SetupNode() override
            {
                name = "Passthrough";
                AddInput<Ref<Texture>, 1, 4>({"In"});
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
            }

          private:
            Ref<Texture> m_Texture;
        };

        class Output : public TextureNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Output);

          public:
            Output(const std::vector<std::string>& outputs = {"Output"}) : m_Outputs(outputs) {}
            virtual ~Output() = default;

            void Process() override
            {
                for (size_t i = 0; i < m_Textures.size(); i++)
                {
                    const Ref<Texture>* ref = GetInputDataPtr<Ref<Texture>>(i);
                    if (ref == nullptr) { m_Textures[i] = nullptr; }
                    else
                    {
                        auto r = *ref;
                        TextureSettings settings;
                        settings.wrapping = TextureWrapping::Repeat;
                        m_Textures[i]     = TextureManager::CopyTexture(r, r->GetFormat(), settings);
                    }
                }
            }

            Ref<Texture> GetTexture() const override { return nullptr; }

            void DrawFields() override {}

            Ref<Texture> GetTexture(size_t index) const { return index < m_Textures.size() ? m_Textures[index] : nullptr; }

            void SetupNode() override
            {
                name = "Output";
                for (const auto& o : m_Outputs)
                {
                    AddInput<Ref<Texture>, 1, 4>({o});
                }
                m_Textures.resize(m_Outputs.size());
            }

            void Serialize(DataObject& data) override
            {
                data.Serialize("Outputs", m_Outputs);
                TextureNode::Serialize(data);
                processed &= !data.changed;
            }

          private:
            std::vector<Ref<Texture>> m_Textures;
            std::vector<std::string> m_Outputs;
        };

        class Input : public TextureNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Input);

          public:
            Input()          = default;
            virtual ~Input() = default;

            void Process() override
            {
                m_Texture = AssetManager::Load<Texture>(m_Path);
                processed = m_Texture != nullptr;

                if (m_Texture != nullptr)
                {
                    auto channels = GetTextureChannels(m_Texture->GetFormat());
                    if (channels == 1) SetOutputType<Ref<Texture>, 1>(0);
                    if (channels == 4) SetOutputType<Ref<Texture>, 4>(0);
                }
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            void SetupNode() override
            {
                name = "Input";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
            }

            void Serialize(DataObject& data) override
            {
                TextureNode::Serialize(data);

                data.Serialize("Path", m_Path);

                processed &= !data.changed;
            }

          private:
            Ref<Texture> m_Texture;
            std::filesystem::path m_Path;
        };

        class ComputeNode : public TextureNode
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
                    if (!IsInputType<Ref<Texture>, 1>(i) && !IsInputType<Ref<Texture>, 4>(i) && !IsInputType<Ref<Texture>, 1, 4>(i))
                        continue;

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
                TextureNode::DrawFields();
                ImGui::Text("Time: %5.2f ms", m_ProcessTime.count() / 1000.0f);
            }

            void Serialize(DataObject& data) override
            {
                TextureNode::Serialize(data);

                ImGui::Text("Time: %5.2f ms", m_ProcessTime.count() / 1000.0f);
                if (ImGui::Button("Reload shaders"))
                {
                    m_Shader = AssetManager::Load<Shader>(std::string(ShaderPath) + m_ShaderName, false);
                    DoProcess();
                }
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            virtual void SetUniforms() const {};

            void SetShader(const std::string& name)
            {
                if (m_ShaderName == name) return;

                m_ShaderName = name;
                m_Shader     = AssetManager::Load<Shader>(std::string(ShaderPath) + name);
            }

            void SetTexture(TextureFormat format)
            {
                if (m_Texture != nullptr && m_Texture->GetFormat() == format) return;
                
                if (m_Texture == nullptr) 
                { 
                    TextureSettings settings;
                    settings.wrapping = TextureWrapping::MirroredRepeat;
                    m_Texture = TextureManager::CreateTexture(TextureSize, TextureSize, format, settings); 
                }
                else
                {
                    m_Texture = TextureManager::CreateTexture(TextureSize, TextureSize, format, m_Texture->GetSettings());
                }
            }

            void SetTexture(TextureFormat format, TextureSettings settings)
            {
                if (m_Texture != nullptr && m_Texture->GetFormat() == format && m_Texture->GetSettings() == settings) return;
                m_Texture = TextureManager::CreateTexture(TextureSize, TextureSize, format, settings);
            }

            std::chrono::microseconds GetProcessTimeMs() const override { return m_ProcessTime; };

          protected:
            Ref<Texture> m_Texture;
            Ref<Shader> m_Shader;
            std::string m_ShaderName;
            uint32_t m_WorkGroupSize = 16;

            std::chrono::microseconds m_ProcessTime {};
        };
    } // namespace TextureNodes
} // namespace EVA