#pragma once

#include "EVA/Renderer/UniformBuffer.hpp"

namespace EVA
{
    class OpenGLUniformBuffer : public UniformBuffer
    {
      public:
        OpenGLUniformBuffer(uint32_t binding, const void* data, uint32_t size, Usage usage);
        virtual ~OpenGLUniformBuffer();

        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
        virtual void Bind(uint32_t binding) override;

      private:
        uint32_t m_RendererId;
        unsigned int m_Usage;
    };
} // namespace EVA