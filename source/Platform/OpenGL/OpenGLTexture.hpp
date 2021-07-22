#pragma once

#include "EVA/Renderer/Texture.hpp"

namespace EVA
{
    class OpenGLTexture
    {
      public:
        static uint32_t CreateGLTextureId(const Texture& texture, const void* data, const std::string& id = "");
        static uint32_t CreateGLTextureId(const Texture& texture, const std::string& id = "");
        static uint32_t CreateGLCubemapId(const Texture& texture, const std::string& id = "");

        static uint32_t CopyTexture(const Texture& source, const Texture& texture, const std::string& id = "");

        static void GetDataFromGpu(const Texture& texture, void* buffer, uint32_t bufferSize, int level = 0);

        static void DeleteGLTexture(const Texture& texture);

        static void GenerateMipMaps(const Texture& texture);

        static unsigned int GetGLTarget(const TextureTarget value);
        static unsigned int GetGLFormat(const TextureFormat format);
        static unsigned int GetGLDataType(const TextureDataType dataType);
        static unsigned int GetGLPixelDataFormat(const PixelDataFormat value);

        static int GetGLMinFilter(const TextureMinFilter value);
        static int GetGLMagFilter(const TextureMagFilter value);
        static int GetGLWrapping(const TextureWrapping value);
    };
} // namespace EVA
