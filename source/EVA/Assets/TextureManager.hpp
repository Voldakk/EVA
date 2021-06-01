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

        static Ref<Texture> CopyTexture(const Ref<Texture>& source, TextureFormat format, const TextureSettings& settings);

        template<typename T>
        static Ref<GridData<T>> GetDataFromGpu(const Ref<Texture>& texture);

        static void GetDataFromGpu(const Ref<Texture>& texture, void* buffer, uint32_t bufferSize);

        static void Delete(Texture& texture);

        static Ref<RawTexture> LoadRaw(const std::filesystem::path& path);
        static void DeleteRaw(RawTexture& texture);

        static void GenerateMipMaps(Ref<Texture>& texture);
    };

    template<typename T>
    inline Ref<GridData<T>> TextureManager::GetDataFromGpu(const Ref<Texture>& texture)
    {
        EVA_PROFILE_FUNCTION();

        auto data = CreateRef<GridData<T>>(texture->GetWidth(), texture->GetHeight());
        GetDataFromGpu(texture, data->Data(), data->Size());
        return data;
    }
} // namespace EVA
