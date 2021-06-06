#pragma once

#include "Base.hpp"

namespace EVA
{
    namespace TextureNodes
    {
        class Noise : public ComputeNode
        {
          protected:
            Noise()
            {
                SetTexture(TextureR);
            }

            void SetupNode() override
            {
                ComputeNode::SetupNode();
                AddOutput<Ref<Texture>, 1>({"Out", &m_Texture});
            }

            void SetUniforms() const override
            {
                m_Shader->SetUniformFloat("u_Scale", m_Scale);
                m_Shader->SetUniformFloat2("u_Position", m_Position);
            }

            void Serialize(DataObject& data) override
            {
                ComputeNode::Serialize(data);

                data.Serialize("Position", m_Position);
                data.Serialize("Scale", m_Scale);

                processed &= !data.changed;
            }

          private:
            float m_Scale = 10.0f;
            glm::vec2 m_Position {};
        };

        class GradientNoise : public Noise
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::GradientNoise);

          public:
            GradientNoise() : Noise()
            {
                SetShader("noise/gradient.glsl");
            }

            void SetupNode() override
            {
                Noise::SetupNode();
                name = "Gradient noise";
            }

            void SetUniforms() const override
            {
                Noise::SetUniforms();
                m_Shader->SetUniformInt("u_Octaves", m_Octaves);
            }

            void Serialize(DataObject& data) override
            {
                Noise::Serialize(data);
                data.Serialize("Octaves", m_Octaves);
                processed &= !data.changed;
            }

          private:
            int m_Octaves = 4;
        };

        class VoronoiNoise : public Noise
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::VoronoiNoise);

          public:
            VoronoiNoise() : Noise()
            {
                SetShader("noise/voronoi.glsl");
            }

            void SetupNode() override
            {
                Noise::SetupNode();
                name = "Voronoi noise";
            }
        };

        class WorleyNoise : public Noise
        {
            REGISTER_SERIALIZABLE(::EVA::TextureNodes::WorleyNoise);

          public:
            WorleyNoise() : Noise()
            {
                SetShader("noise/worley.glsl");
            }

            void SetupNode() override
            {
                Noise::SetupNode();
                name = "Worley noise";
            }

            void SetUniforms() const override
            {
                Noise::SetUniforms();
                m_Shader->SetUniformFloat("u_Exponent", m_Exponent);
            }

            void Serialize(DataObject& data) override
            {
                Noise::Serialize(data);
                data.Serialize("Exponent", m_Exponent);
                processed &= !data.changed;
            }

          private:
            float m_Exponent = 1.5f;
        };
    } // namespace TextureNodes
} // namespace EVA