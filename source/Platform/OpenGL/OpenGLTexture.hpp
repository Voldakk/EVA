#pragma once

#include "EVA/Renderer/Texture.hpp"

namespace EVA
{
    class OpenGLTexture2D : public Texture2D
    {
        uint32_t m_RendererId;
        uint32_t m_Width, m_Height;
        std::string m_Path;

      public:
        OpenGLTexture2D(const std::string& path);
        virtual ~OpenGLTexture2D();

        inline virtual uint32_t GetWidth() const override { return m_Width; }
        inline virtual uint32_t GetHeight() const override { return m_Height; }

        virtual void Bind(uint32_t slot = 0) const override;
    };
} // namespace EVA
