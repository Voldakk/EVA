#include "OpenGLTexture.hpp"

#include <glad/glad.h>
#include <stb_image.h>

namespace EVA
{
    OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : m_Path(path)
    {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        auto data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        EVA_INTERNAL_ASSERT(data != nullptr, "Falied to load image");
        m_Width  = width;
        m_Height = height;

        GLenum internalFormat = 0, dataFormat = 0;
        switch (channels)
        {
            case 3:
                internalFormat = GL_RGB8;
                dataFormat     = GL_RGB;
                break;
            case 4:
                internalFormat = GL_RGBA8;
                dataFormat     = GL_RGBA;
                break;
        }

        EVA_INTERNAL_ASSERT(internalFormat != 0, "Format not supported");

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
        glTextureStorage2D(m_RendererId, 1, internalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    OpenGLTexture2D::OpenGLTexture2D(const uint32_t width, const uint32_t height) { Resize(width, height); }

    OpenGLTexture2D::~OpenGLTexture2D() { glDeleteTextures(1, &m_RendererId); }

    void OpenGLTexture2D::Resize(const uint32_t width, const uint32_t height)
    {
        m_Width  = width;
        m_Height = height;

        if (m_RendererId != 0) { glDeleteTextures(1, &m_RendererId); }

        glGenTextures(1, &m_RendererId);
        glBindTexture(GL_TEXTURE_2D, m_RendererId);

        glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }


} // namespace EVA