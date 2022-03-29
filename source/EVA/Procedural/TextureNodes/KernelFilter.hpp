#pragma once

#include "Base.hpp"

namespace EVA
{
    namespace TextureNodes
    {
        class KernelFilter : public ComputeNode
        {
          public:
            KernelFilter()
            {
                SetShader("kernel_filter/kernel_filter.glsl");
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