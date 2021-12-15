#pragma once

#include "Base.hpp"
#include <imgui_color_gradient.h>
#include <imgui_curve_editor.hpp>

namespace EVA
{
    namespace TextureNodes
    {
        class Blend : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Blend);

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
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1, 4>({"A"});
                AddInput<Ref<Texture>, 1, 4>({"B"});
                AddInput<Ref<Texture>, 1>({"Opacity", &TextureWhite()});
            }

            bool ProcessTextureNode() override
            {
                if (GetInputDataType(0) != GetInputDataType(1)) { return false; }

                if (IsInputDataType<Ref<Texture>, 1>(0))
                {
                    SetShader("blend_grayscale.glsl");
                    SetTexture(TextureR);
                    SetOutputType<Ref<Texture>, 1>(0);
                }
                else
                {
                    SetShader("blend_rgba.glsl");
                    SetTexture(TextureRGBA);
                    SetOutputType<Ref<Texture>, 4>(0);
                }

                return ComputeNode::ProcessTextureNode();
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
                    data.changed |= ImGui::SliderFloat("Opacity", &m_Opacity, 0.0f, 1.0f);

                    const char* items[] = {"Copy",   "Add",     "Substract",  "Multiply",   "Divide",     "Darken",      "Lighten",
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

        class Levels : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Levels);

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
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1, 4>({"In"});
            }

            bool ProcessTextureNode() override
            {
                if (IsInputDataType<Ref<Texture>, 1>(0))
                {
                    SetShader("levles_grayscale.glsl");
                    SetTexture(TextureR);
                    SetOutputType<Ref<Texture>, 1>(0);
                }
                else
                {
                    SetShader("levles_rgba.glsl");
                    SetTexture(TextureRGBA);
                    SetOutputType<Ref<Texture>, 4>(0);
                }

                return ComputeNode::ProcessTextureNode();
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
            int m_SelectedChannel = 0;

            glm::vec2 m_InputRange  = {0.0f, 1.0f};
            glm::vec2 m_OutputRange = {0.0f, 1.0f};
            float m_Midtone         = 0.5f;

            glm::vec4 m_InputRangeMin = glm::vec4(0.0f);
            glm::vec4 m_InputRangeMax = glm::vec4(1.0f);

            glm::vec4 m_OutputRangeMin = glm::vec4(0.0f);
            glm::vec4 m_OutputRangeMax = glm::vec4(1.0f);

            glm::vec4 m_Midtones = glm::vec4(0.5f);
        };

        class GradientMap : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::GradientMap);

          public:
            GradientMap()
            {
                SetShader("gradientmap.glsl");
                SetTexture(TextureRGBA);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Gradient map";
                AddOutput<Ref<Texture>, 4>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
            }

            void SetUniforms() const override
            {
                int i = 0;
                for (auto markIt = m_Gradient.getMarks().begin(); markIt != m_Gradient.getMarks().end(); ++markIt)
                {
                    ImGradientMark mark = **markIt;
                    auto u              = "u_Marks[" + std::to_string(i) + "].";
                    m_Shader->SetUniformFloat(u + "position", mark.position);
                    m_Shader->SetUniformFloat4(u + "color", glm::vec4(mark.color[0], mark.color[1], mark.color[2], mark.color[3]));
                    i++;
                }
                m_Shader->SetUniformInt("u_Count", m_Gradient.getMarks().size());
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector())
                {
                    static bool show                    = false;
                    static ImGradientMark* draggingMark = nullptr;
                    static ImGradientMark* selectedMark = nullptr;
                    if (ImGui::GradientButton(&m_Gradient)) { show = !show; }
                    if (show) { data.changed |= ImGui::GradientEditor(&m_Gradient, draggingMark, selectedMark); }
                }
                else if (data.Save())
                {
                    auto marks = m_Gradient.getSerializeableMarks();
                    data.Serialize("marks", marks);
                }
                else if (data.Load())
                {
                    ImGradient::MarksVector marks;
                    data.Serialize("marks", marks);
                    m_Gradient.setFromSerializeableMarks(marks);
                }

                processed &= !data.changed;
            }

          private:
            ImGradient m_Gradient;
        };

        class DirectionalWarp : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::DirectionalWarp);

          public:
            DirectionalWarp()
            {
                SetShader("directional_warp.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Directional warp";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
                AddInput<Ref<Texture>, 1>({"Intensity", &TextureWhite()});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat("u_Angle", m_Angle);
                m_Shader->SetUniformFloat("u_Intensity", m_Intensity);

                const Ref<Texture>& ref = GetInputData<Ref<Texture>>(0);
                if (ref)
                    m_Shader->BindTexture("u_InputMapSampler", ref);
                else
                    m_Shader->BindTexture("u_InputMapSampler", TextureTarget::Texture2D, 0);
            }


            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector()) { data.changed |= ImGui::SliderFloat("Angle", &m_Angle, 0, 360); }
                else
                {
                    data.Serialize("Angle", m_Angle);
                }
                data.Serialize("Intensity", m_Intensity);

                processed &= !data.changed;
            }

          private:
            float m_Angle     = 0;
            float m_Intensity = 1;
        };

        class NonUniformDirectionalWarp : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::NonUniformDirectionalWarp);

          public:
            NonUniformDirectionalWarp()
            {
                SetShader("non_uniform_directional_warp.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Directional warp";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
                AddInput<Ref<Texture>, 1>({"Intensity", &TextureWhite()});
                AddInput<Ref<Texture>, 1>({"Angle", &TextureWhite()});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat("u_Angle", m_Angle);
                m_Shader->SetUniformFloat("u_AngleMultiplier", m_AngleMultiplier);
                m_Shader->SetUniformFloat("u_Intensity", m_Intensity);

                const Ref<Texture>& ref = GetInputData<Ref<Texture>>(0);
                if (ref)
                    m_Shader->BindTexture("u_InputMapSampler", ref);
                else
                    m_Shader->BindTexture("u_InputMapSampler", TextureTarget::Texture2D, 0);
            }


            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector())
                {
                    data.changed |= ImGui::SliderFloat("Angle", &m_Angle, 0, 360);
                    data.changed |= ImGui::SliderFloat("Angle multiplier", &m_AngleMultiplier, 0, 1);
                }
                else
                {
                    data.Serialize("Angle", m_Angle);
                    data.Serialize("Angle multiplier", m_AngleMultiplier);
                }
                data.Serialize("Intensity", m_Intensity);

                processed &= !data.changed;
            }

          private:
            float m_Angle           = 0;
            float m_AngleMultiplier = 1;
            float m_Intensity       = 1;
        };

        class NormalBlend : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::NormalBlend);

          public:
            NormalBlend()
            {
                SetShader("normal_blend.glsl");
                SetTexture(TextureRGBA);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Normal blend";
                AddOutput<Ref<Texture>, 4>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 4>({"A"});
                AddInput<Ref<Texture>, 4>({"B"});
                AddInput<Ref<Texture>, 1>({"Opacity", &TextureWhite()});
            }

            void SetUniforms() const override { m_Shader->SetUniformFloat("u_Opacity", m_Opacity); }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector()) { data.changed |= ImGui::SliderFloat("Opacity", &m_Opacity, 0, 1); }
                else
                {
                    data.Serialize("Opacity", m_Opacity);
                }

                processed &= !data.changed;
            }

          private:
            float m_Opacity = 1.0f;
        };

        class Curve : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Curve);

          public:
            Curve()
            {
                SetShader("curve.glsl");
                SetTexture(TextureR);
                points[0] = {0.4f, 0.4f};
                points[1] = {0.5f, 0.5f};
                points[2] = {0.6f, 0.6f};
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Normal blend";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"A"});
            }

            void SetUniforms() const override {}

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                struct Point
                {
                    float x, y;
                };

                if (data.Inspector())
                {
                    int newCount;
                    ImVec2 size = {-1, 200};
                    int flags   = (int)ImGui::CurveEditorFlags::SHOW_GRID | (int)ImGui::CurveEditorFlags::NO_TANGENTS;
                    // flags       = 0;
                    /*for (size_t i = 0; i < m_Points.size(); i++)
                    {
                        points[i].x = m_Points[i].x;
                        points[i].y = m_Points[i].y;
                    }*/

                    static bool a = true;
                    if (a)
                    {
                        a     = false;
                        flags = flags | (int)ImGui::CurveEditorFlags::RESET;
                    }
                    int changed = ImGui::CurveEditor("Curve_123", (float*)points, count, size, flags, &newCount);

                    if (changed >= 0)
                    {
                        int a;
                        // m_Points[changed].x = points[changed].x;
                        // m_Points[changed].y = points[changed].y;
                    }

                    if (newCount != count)
                    {
                        count = newCount;
                        // m_Points.resize(newCount);
                        // m_Points[i].x = points[i].x;
                        // m_Points[i].y = points[i].y;
                    }
                }
                else
                {
                }

                processed &= !data.changed;
            }

          private:
            // std::vector<glm::vec2> m_Points {{0.0f, 0.0f}, {0.2f, 0.1f} , {0.5f, 0.6f}, {1.0f, 1.0f}};
            ImVec2 points[16];
            int count = 3;
        };

        class KernelFilter : public ComputeNode
        {
          public:
            KernelFilter()
            {
                SetShader("kernel_filter.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
                AddInput<Ref<Texture>, 1>({"In"});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformInt("u_StepSize", m_StepSize);

                auto kernel = GetKernel();
                if (!kernel.empty())
                {
                    glm::ivec2 size = {kernel.size(), kernel[0].size()};
                    m_Shader->SetUniformInt2("u_KernelSize", size);
                    m_Shader->SetUniformInt("u_Divisor", GetDivisor());

                    for (size_t y = 0; y < size.y; y++)
                    {
                        for (size_t x = 0; x < size.x; x++)
                        {
                            m_Shader->SetUniformInt("u_Kernel[" + std::to_string(x + y * size.x) + "]", kernel[y][x]);
                        }
                    }
                }
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                data.Serialize("Step size", m_StepSize);

                processed &= !data.changed;
            }

            virtual const std::vector<std::vector<int32_t>>& GetKernel() const = 0;
            virtual int32_t GetDivisor() const                                 = 0;

          private:
            int32_t m_StepSize = 1;
        };

        class GaussianBlur5 : public KernelFilter
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::GaussianBlur5);

          public:
            void SetupNode() override
            {
                KernelFilter::SetupNode();
                name = "Gaussian blur 5x5";
            }

            virtual const std::vector<std::vector<int32_t>>& GetKernel() const override { return m_kernel; }
            virtual int32_t GetDivisor() const override { return 256; }

          private:
            std::vector<std::vector<int32_t>> m_kernel {{1, 4, 6, 4, 1}, {4, 16, 24, 16, 4}, {6, 24, 36, 24, 6}, {4, 16, 24, 16, 4}, {1, 4, 6, 4, 1}};
        };
    } // namespace TextureNodes
} // namespace EVA