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
        OpenGLTexture2D(const uint32_t width, const uint32_t height);
        virtual ~OpenGLTexture2D();

        virtual void Resize(const uint32_t width, const uint32_t height) override;

        inline virtual uint32_t GetWidth() const override { return m_Width; }
        inline virtual uint32_t GetHeight() const override { return m_Height; }
        inline virtual uint32_t GetRendererId() const override { return m_RendererId; }
    };
} // namespace EVA
