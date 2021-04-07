#pragma once

#include "EVA/Renderer/ShaderStorageBuffer.hpp"

namespace EVA
{
    class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
    {
      public:
        OpenGLShaderStorageBuffer(void* data, uint32_t size);
        ~OpenGLShaderStorageBuffer();
        virtual void BufferData(void* data, uint32_t size) override;

        inline virtual uint32_t GetRendererId() const override { return m_RendererId; }

      private:
        uint32_t m_RendererId;
        uint32_t m_Size;
    };
} // namespace EVA
