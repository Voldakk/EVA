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
                AddInputs<Ref<Texture>, 1, 4>({{"In"}});
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}});
            }

          private:
            Ref<Texture> m_Texture;
        };

        class Output : public TextureNode
        {
          public:
            Output(const std::string& n = "Output") { name = n; }
            virtual ~Output() = default;

            void Process() override
            {
                const Ref<Texture>* ref = GetInputDataPtr<Ref<Texture>>(0);
                m_Texture               = *ref;
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            void SetupNode() override
            {
                //name = "Output";
                AddInputs<Ref<Texture>, 1, 4>({{"In"}});
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
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}});
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
                    if (!IsInputType<Ref<Texture>, 1>(i) && !IsInputType<Ref<Texture>, 4>(i) && !IsInputType<Ref<Texture>, 1, 4>(i)) continue;

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
                if (ImGui::Button("Reload shaders")) 
                { 
                    m_Shader = Shader::Create(std::string(ShaderPath) + m_ShaderName);
                    DoProcess();
                }
            }

            Ref<Texture> GetTexture() const override { return m_Texture; }

            virtual void SetUniforms() const {};

            void SetShader(const std::string& name)
            {
                if (m_ShaderName == name) return;

                m_ShaderName = name;
                m_Shader = Shader::Create(std::string(ShaderPath) + name);
            }

            void SetTexture(TextureFormat format) 
            { 
                if (m_Texture != nullptr && m_Texture->GetFormat() == format) return;
                m_Texture = TextureManager::CreateTexture(512, 512, format);
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
                SetTexture(TextureFormat::R32F);
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
                SetTexture(TextureFormat::R32F);
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

        class Blend : public ComputeNode
        {
          public:
            Blend()
            {
                // SetShader("blend_grayscale.glsl");
                // SetTexture(TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Blend";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}}); 
                AddInputs<Ref<Texture>, 1, 4>({{"A"}, {"B"}});
                AddInputs<float>({{"Frac", false}});
            }

            void Process() override
            {
                if (GetInputDataType(0) != GetInputDataType(1)) 
                { 
                    processed = false;
                    return;
                }

                if (IsInputDataType<Ref<Texture>, 1>(0)) 
                {
                    SetShader("blend_grayscale.glsl");
                    SetTexture(TextureFormat::R32F);
                    SetOutputType<Ref<Texture>, 1>(0);
                }
                else
                {
                    SetShader("blend_rgba.glsl");
                    SetTexture(TextureFormat::RGBA32F);
                    SetOutputType<Ref<Texture>, 4>(0);
                }

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
                SetTexture(TextureFormat::R32F);
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

        class Levels : public ComputeNode
        {
          public:
            Levels()
            {
                // SetShader("levles_grayscale.glsl");
                // SetTexture(TextureFormat::R32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Levels";
                AddOutputs<Ref<Texture>, 1>({{"Out", &m_Texture}});
                AddInputs<Ref<Texture>, 1, 4>({{"In"}});
            }

            void Process() override
            {
                if (IsInputDataType<Ref<Texture>, 1>(0))
                {
                    SetShader("levles_grayscale.glsl");
                    SetTexture(TextureFormat::R32F);
                    SetOutputType<Ref<Texture>, 1>(0);
                }
                else
                {
                    SetShader("levles_rgba.glsl");
                    SetTexture(TextureFormat::RGBA32F);
                    SetOutputType<Ref<Texture>, 4>(0);
                }

                ComputeNode::Process();
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat2("u_InputRange", m_InputRange);
                m_Shader->SetUniformFloat2("u_OutputRange", m_OutputRange);
                m_Shader->SetUniformFloat("u_Midtone", m_Midtone);

                if (m_Texture != nullptr && GetTextureChannels(m_Texture->GetFormat()) == 4)
                {
                    m_Shader->SetUniformFloat4("u_InputRangeMin", m_InputRangeMin);
                    m_Shader->SetUniformFloat4("u_InputRangeMax", m_InputRangeMax);
                    m_Shader->SetUniformFloat4("u_OutputRangeMin", m_OutputRangeMin);
                    m_Shader->SetUniformFloat4("u_OutputRangeMax", m_OutputRangeMax);
                    m_Shader->SetUniformFloat4("u_Midtones", m_Midtones);
                }
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector())
                {
                    ImGui::Text("Key");
                    data.changed |= ImGui::SliderFloat2("Input range", glm::value_ptr(m_InputRange), 0.0f, 1.0f);
                    data.changed |= ImGui::SliderFloat2("Output range", glm::value_ptr(m_OutputRange), 0.0f, 1.0f);
                    data.changed |= ImGui::SliderFloat("Midtone", &m_Midtone, 0.0f, 1.0f);

                    if (m_Texture != nullptr && GetTextureChannels(m_Texture->GetFormat()) == 4)
                    {
                        ImGui::PushID(0);
                        ImGui::Separator();

                        const char* items[] = {"R", "G", "B", "A"};
                        ImGui::Combo("Blend mode", &m_SelectedChannel, items, IM_ARRAYSIZE(items));

                        glm::vec2 inputRange  = {m_InputRangeMin[m_SelectedChannel], m_InputRangeMax[m_SelectedChannel]};
                        glm::vec2 outputRange = {m_OutputRangeMin[m_SelectedChannel], m_OutputRangeMax[m_SelectedChannel]};
                        float midtone         = m_Midtones[m_SelectedChannel];
                        data.changed |= ImGui::SliderFloat2("Input range", glm::value_ptr(inputRange), 0.0f, 1.0f);
                        data.changed |= ImGui::SliderFloat2("Output range", glm::value_ptr(outputRange), 0.0f, 1.0f);
                        data.changed |= ImGui::SliderFloat("Midtone", &midtone, 0.0f, 1.0f);
                        m_InputRangeMin[m_SelectedChannel]  = inputRange.x;
                        m_InputRangeMax[m_SelectedChannel]  = inputRange.y;
                        m_OutputRangeMin[m_SelectedChannel] = outputRange.x;
                        m_OutputRangeMax[m_SelectedChannel] = outputRange.y;
                        m_Midtones[m_SelectedChannel]       = midtone;

                        ImGui::PopID();
                    }
                }
                else
                {
                    data.Serialize("m_SelectedChannel", m_SelectedChannel);

                    data.Serialize("m_InputRange", m_InputRange);
                    data.Serialize("m_OutputRange", m_OutputRange);
                    data.Serialize("m_Midtone", m_Midtone);

                    data.Serialize("m_InputRangeMin", m_InputRangeMin);
                    data.Serialize("m_InputRangeMax", m_InputRangeMax);

                    data.Serialize("m_OutputRangeMin", m_OutputRangeMin);
                    data.Serialize("m_OutputRangeMax", m_OutputRangeMax);

                    data.Serialize("m_Midtones", m_Midtones);
                }

                processed &= !data.changed;
            }

          private:
            int m_SelectedChannel   = 0;

            glm::vec2 m_InputRange  = {0.0f, 1.0f};
            glm::vec2 m_OutputRange = {0.0f, 1.0f};
            float m_Midtone         = 0.5f;

            glm::vec4 m_InputRangeMin  = glm::vec4(0.0f);
            glm::vec4 m_InputRangeMax = glm::vec4(1.0f);

            glm::vec4 m_OutputRangeMin = glm::vec4(0.0f);
            glm::vec4 m_OutputRangeMax = glm::vec4(1.0f);

            glm::vec4 m_Midtones = glm::vec4(0.5f);
        };

        class GradientMap : public ComputeNode
        {
          public:
            GradientMap()
            {
                SetShader("gradientmap.glsl");
                SetTexture(TextureFormat::RGBA32F);
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

        class Uniform : public ComputeNode
        {
          public:
            Uniform()
            {
                //SetShader("uniform_rgba.glsl");
                //SetShader("uniform_grayscale.glsl");
                //SetTexture(TextureFormat::RGBA32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Uniform color";
                AddOutputs<Ref<Texture>, 4>({{"Out", &m_Texture}});
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

        class HeightToNormal : public ComputeNode
        {
          public:
            HeightToNormal()
            {
                SetShader("height_to_normal.glsl");
                SetTexture(TextureFormat::RGBA32F);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Height to Normal";
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
    } // namespace TextureNodes
} // namespace EVA