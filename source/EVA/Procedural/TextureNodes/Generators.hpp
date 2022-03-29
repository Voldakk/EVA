#pragma once

#include "Base.hpp"

namespace EVA
{
    namespace TextureNodes
    {
        class Uniform : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Uniform);

          public:
            Uniform() {}

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Uniform color";
                AddOutput<Ref<Texture>, 4>({"Out", &m_Texture});
            }

            bool ProcessTextureNode() override
            {
                if (m_Mode == 0)
                {
                    SetShader("generators/uniform_grayscale.glsl");
                    SetTexture(TextureR);
                    SetOutputType<Ref<Texture>, 1>(0);
                }
                else
                {
                    SetShader("generators/uniform_rgba.glsl");
                    SetTexture(TextureRGBA);
                    SetOutputType<Ref<Texture>, 4>(0);
                }

                return ComputeNode::ProcessTextureNode();
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

        class Bricks : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Bricks);

          public:
            Bricks()
            {
                SetShader("generators/brick.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Bricks";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
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

        class TileGenerator : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::TileGenerator);

          public:
            TileGenerator()
            {
                SetShader("generators/tile.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Tile Generator";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformInt2("u_Count", m_Count);
                m_Shader->SetUniformFloat2("u_Height", m_Height);

                m_Shader->SetUniformFloat("u_Scale", m_Scale);
                m_Shader->SetUniformFloat("u_ScaleVariation", m_ScaleVariation);

                m_Shader->SetUniformFloat("u_Offset", m_Offset);
                m_Shader->SetUniformFloat("u_OffsetVariation", m_OffsetVariation);

                m_Shader->SetUniformFloat("u_RandomMask", m_RandomMask);

                m_Shader->SetUniformFloat("u_Seed", m_Seed);
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                data.Serialize("Count", m_Count);
                data.Serialize("Height", m_Height);

                if (data.Inspector())
                {
                    data.changed |= ImGui::SliderFloat("Scale", &m_Scale, 0.0f, 1.0f);
                    data.changed |= ImGui::SliderFloat("Scale Variation", &m_ScaleVariation, 0.0f, 1.0f);
                    data.changed |= ImGui::SliderFloat("Offset", &m_Offset, 0.0f, 1.0f);
                    data.changed |= ImGui::SliderFloat("Offset Variation", &m_OffsetVariation, 0.0f, 1.0f);

                    data.changed |= ImGui::SliderFloat("Random Mask", &m_RandomMask, 0.0f, 1.0f);
                }
                else
                {
                    data.Serialize("m_Scale", m_Scale);
                    data.Serialize("m_ScaleVariation", m_ScaleVariation);

                    data.Serialize("m_Offset", m_Offset);
                    data.Serialize("m_OffsetVariation", m_OffsetVariation);

                    data.Serialize("m_RandomMask", m_RandomMask);
                }

                data.Serialize("Seed", m_Seed);

                processed &= !data.changed;
            }

          private:
            glm::ivec2 m_Count = {4, 8};
            glm::vec2 m_Height = {1.0f, 1.0f};

            float m_Scale          = 0.0f;
            float m_ScaleVariation = 0.0f;

            float m_Offset          = 0.0f;
            float m_OffsetVariation = 0.0f;

            float m_RandomMask = 0.0f;

            float m_Seed = 0.0f;
        };

        class Shape : public ComputeNode
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::Shape);

          public:
            Shape()
            {
                SetShader("generators/shape.glsl");
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                name = "Shape";
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformInt("u_Shape", m_Shape);


                m_Shader->SetUniformInt2("u_Count", m_Count);
                m_Shader->SetUniformFloat("u_Scale", m_Scale);

                m_Shader->SetUniformFloat2("u_Size", m_Size);
                m_Shader->SetUniformFloat("u_Angle", glm::radians(m_Angle));

                m_Shader->SetUniformFloat("u_Param", m_Param);
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                if (data.Inspector())
                {
                    const char* items[] = {"Square", "Disc", "Gaussian"};
                    data.changed |= ImGui::Combo("Shape", &m_Shape, items, IM_ARRAYSIZE(items));
                }
                else
                {
                    data.Serialize("Shape", m_Shape);
                }

                data.Serialize("Count", m_Count);

                if (data.Inspector())
                {
                    data.changed |= ImGui::SliderFloat("Scale", &m_Scale, 0.0f, 1.0f);
                    data.changed |= ImGui::SliderFloat2("Size", glm::value_ptr(m_Size), 0.0f, 1.0f);
                    data.changed |= ImGui::SliderFloat("Angle", &m_Angle, 0.0f, 360.0f);
                }
                else
                {
                    data.Serialize("Scale", m_Scale);
                    data.Serialize("Size", m_Size);
                    data.Serialize("Angle", m_Angle);
                }

                data.Serialize("Param", m_Param);

                processed &= !data.changed;
            }

          private:

            int m_Shape = 0;

            glm::ivec2 m_Count = {1, 1};

            float m_Scale    = 1.0f;
            glm::vec2 m_Size = {1.0f, 1.0f};
            float m_Angle    = 0.0f;

            float m_Param = 0.5f;
        };

    } // namespace TextureNodes
} // namespace EVA