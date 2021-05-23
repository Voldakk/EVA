#pragma once

#include "EVA/Renderer/Texture.hpp"

namespace EVA
{
    class TextureManager
    {
        inline static std::unordered_map<std::string, WeakRef<Texture>> s_Textures       = {};
        inline static std::unordered_map<std::string, WeakRef<RawTexture>> s_RawTextures = {};

      public:
        static Ref<Texture> LoadTexture(const std::filesystem::path& path, const TextureSettings& settings);

        static Ref<Texture> CreateTexture(uint32_t width, uint32_t height, TextureFormat format, const TextureSettings& settings = DefaultTextureSettings);
        static Ref<Texture> CreateTexture(uint32_t width, uint32_t height, const void* data, TextureFormat format, const TextureSettings& settings = DefaultTextureSettings);
        static Ref<Texture> CreateCubeMap(uint32_t width, uint32_t height, TextureFormat format, const TextureSettings& settings = DefaultTextureSettings);

        static void Delete(Texture& texture);

        static Ref<RawTexture> LoadRaw(const std::filesystem::path& path);
        static void DeleteRaw(RawTexture& texture);

        static void GenerateMipMaps(Ref<Texture>& texture);
    };
} // namespace EVA
