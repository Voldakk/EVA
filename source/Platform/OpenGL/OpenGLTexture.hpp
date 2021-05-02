#pragma once

#include "EVA/Renderer/Texture.hpp"

#include <glad/glad.h>

namespace EVA
{
    class OpenGLTexture
    {
      public:
        static uint32_t CreateGLTextureId(const Texture& texture, void* data, const std::string& id = "");
        static uint32_t CreateGLTextureId(const Texture& texture, const std::string& id = "");
        static uint32_t OpenGLTexture::CreateGLCubemapId(const Texture& texture, const std::string& id = "");

        static void DeleteGLTexture(const Texture& texture);

        static void GenerateMipMaps(const Texture& texture);

        static GLenum GetGLTarget(const TextureTarget value);

        static GLenum GetGLFormat(const TextureFormat format);
        static GLenum GetGLDataType(const TextureDataType dataType);

        static GLint GetGLMinFilter(const TextureMinFilter value);
        static GLint GetGLMagFilter(const TextureMagFilter value);
        static GLint GetGLWrapping(const TextureWrapping value);
        static GLenum GetGLAccess(const TextureAccess value);
    };
} // namespace EVA
