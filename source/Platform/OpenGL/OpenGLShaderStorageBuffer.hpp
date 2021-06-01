#pragma once

#include "EVA/Renderer/ShaderStorageBuffer.hpp"

namespace EVA
{
    class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
    {
      public:
        OpenGLShaderStorageBuffer(const void* data, uint32_t size, Usage usage);
        ~OpenGLShaderStorageBuffer();
        virtual void BufferData(const void* data, uint32_t size, uint32_t offset) override;

        inline virtual uint32_t GetRendererId() const override { return m_RendererId; }

        virtual void* Map(Access access) const override;
        virtual void Unmap() const override;

      private:
        uint32_t m_RendererId;
        uint32_t m_Size;
        unsigned int m_Usage;
    };
} // namespace EVA
