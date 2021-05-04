#pragma once

#include "EVA.hpp"
#include "NodeEditor.hpp"
#include <chrono>
#include "imgui_color_gradient.h"

namespace EVA
{
    namespace TextureNodes
    {
        constexpr std::string_view ShaderPath = "./assets/procedural/shaders/";

        class TextureNode : public NE::Node
        {
          public:
            TextureNode()       = default;
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
          public:
            Passthrough()          = default;
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

        class Output : public TextureNode
        {
          public:
            Output()          = default;
            virtual ~Output() = default;

            void Process() override
            {
                const Ref<Texture>* ref = GetInputDataPtr<Ref<Texture>>(0);
                m_Texture               = *ref;
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            void SetupNode() override
            {
                name = "Output";
                AddInputs<Ref<Texture>, 1>({{"In"}});
            }

          private:
            Ref<Texture> m_Texture;
        };

        class Input : public TextureNode
        {
          public:
            Input()           = default;
            virtual ~Input() = default;

            void Process() override
            {
                m_Texture = TextureManager::LoadTexture(m_Path);
                processed = m_Texture != nullptr;
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            void SetupNode() override
            {
                name = "Input";
                AddOutputs<Ref<Texture>, 1>({{"Gray", &m_Texture}});
                AddOutputs<Ref<Texture>, 4>({{"RGBA", &m_Texture}});
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
                TextureNode::DrawFields();
                ImGui::Text("Time: %6.2f ms", m_ProcessTime.count() / 1000.0f);
            }

            void Serialize(DataObject& data) override 
            {
                TextureNode::Serialize(data);

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

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat("u_Scale", m_Scale);
                m_Shader->SetUniformFloat2("u_Position", m_Position);
            }

            void Serialize(DataObject& data) override 
            {
                ComputeNode::Serialize(data);

                data.Serialize("Scale", m_Scale);
                data.Serialize("Position", m_Position);

                processed &= !data.changed;
            }

          private:
            float m_Scale = 10.0f;
            glm::vec2 m_Position {};
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

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat("u_Scale", m_Scale);
                m_Shader->SetUniformFloat2("u_Position", m_Position);
                m_Shader->SetUniformInt("u_Octaves", m_Octaves);
            }

            void Serialize(DataObject& data) override 
            {
                ComputeNode::Serialize(data);

                data.Serialize("Position", m_Position);
                data.Serialize("Scale", m_Scale);
                data.Serialize("Octaves", m_Octaves);

                processed &= !data.changed;
            }

          private:
            float m_Scale = 10.0f;
            glm::vec2 m_Position {};
            int m_Octaves = 4;

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

            void Serialize(DataObject& data) override 
            {
                ComputeNode::Serialize(data);

                if (data.Inspector())
                {
                    if (!InputConnected(2)) { data.changed |= ImGui::SliderFloat("Opacity", &m_Opacity, 0.0f, 1.0f); }

                    const char* items[] = {"Copy", "Add", "Substract", "Multiply", "Divide", "Darken", "Lighten", 
                        "Screen", "Overlay", "Hard Light", "Soft Light", "Difference", "Color Dodge", "Color Burn"};
                    data.changed |= ImGui::Combo("Blend mode", &m_BlendMode, items, IM_ARRAYSIZE(items));
                }
                else
                {
                    data.Serialize("Opacity", m_Opacity);
                    data.Serialize("BlendMode", m_BlendMode);
                }

                processed &= !data.changed;
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

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector())
                {
                    data.changed |= ImGui::SliderFloat2("Input range", glm::value_ptr(m_InputRange), 0.0f, 1.0f);
                    data.changed |= ImGui::SliderFloat2("Output range", glm::value_ptr(m_OutputRange), 0.0f, 1.0f);
                    data.changed |= ImGui::SliderFloat("Midtone", &m_Midtone, 0.0f, 1.0f);
                }
                else
                {
                    data.Serialize("Input range", m_InputRange);
                    data.Serialize("Output range", m_OutputRange);
                    data.Serialize("Midtone", m_Midtone);
                }

                processed &= !data.changed;
            }

          private:
            glm::vec2 m_InputRange  = {0.0f, 1.0f};
            glm::vec2 m_OutputRange = {0.0f, 1.0f};
            float m_Midtone         = 0.5f;
        };

        class GradientMap : public ComputeNode
        {
          public:
            GradientMap()
            {
                SetShader("gradientmap.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::RGBA32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Gradient map";
                AddOutputs<Ref<Texture>, 4>({{"Out", &m_Texture}});
                AddInputs<Ref<Texture>, 1>({{"In"}});
            }

            void SetUniforms() const override 
            { 
                int i = 0;
                for (auto markIt = m_Gradient.getMarks().begin(); markIt != m_Gradient.getMarks().end(); ++markIt)
                {
                    ImGradientMark mark = **markIt;
                    auto u = "u_Marks[" + std::to_string(i) + "].";
                    m_Shader->SetUniformFloat(u + "position", mark.position);
                    m_Shader->SetUniformFloat4(u + "color", glm::vec4(mark.color[0], mark.color[1], mark.color[2], mark.color[3])); 
                    i++;
                }
                m_Shader->SetUniformInt("u_Count", m_Gradient.getMarks().size());
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                static bool show = false;
                static ImGradientMark* draggingMark = nullptr;
                static ImGradientMark* selectedMark = nullptr;
                if (ImGui::GradientButton(&m_Gradient)) { show = !show; }
                if (show)
                { 
                    data.changed |= ImGui::GradientEditor(&m_Gradient, draggingMark, selectedMark);
                }
                processed &= !data.changed;
            }

          private:
            ImGradient m_Gradient;
        };

        class UniformGrayscale : public ComputeNode
        {
          public:
            UniformGrayscale()
            {
                SetShader("uniform_grayscale.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Uniform grayscale";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}});
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat("u_Value", m_Value); }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector()) { data.changed |= ImGui::SliderFloat("Value", &m_Value, 0.0f, 1.0f); }
                else
                {
                    data.Serialize("Input range", m_Value);
                }

                processed &= !data.changed;
            }

          private:
            float m_Value = 0;
        };

        class UniformColor : public ComputeNode
        {
          public:
            UniformColor()
            {
                SetShader("uniform_color.glsl");
                m_Texture = TextureManager::CreateTexture(512, 512, TextureFormat::RGBA32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Uniform color";
                AddOutputs<Ref<Texture>, 4>({{"Out", &m_Texture}});
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat4("u_Color", m_Color); }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector()) { data.changed |= ImGui::ColorEdit4("Color", glm::value_ptr(m_Color)); }
                else
                {
                    data.Serialize("Input range", m_Color);
                }

                processed &= !data.changed;
            }

          private:
            glm::vec4 m_Color = glm::vec4(0, 0, 0, 1);
        };
    } // namespace TextureNodes
} // namespace EVA