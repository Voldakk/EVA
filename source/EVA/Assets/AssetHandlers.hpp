#pragma once
#include "AssetManager.hpp"
#include "EVA/Renderer/Shader.hpp"
#include "EVA/Renderer/Texture.hpp"
#include "EVA/Assets/Mesh.hpp"

namespace EVA
{
    template<>
    class AssetHandler<Shader>
    {
      public:
        static Ref<Asset> Create() { return Shader::Create(""); }

        static Ref<Asset> Load(const std::filesystem::path& path, DataObject& file, DataObject& metafile) { return Shader::Create(path); }

        static void Save(Ref<Asset>& asset, const std::filesystem::path& path, DataObject& file, DataObject& metafile) {}
    };

    template<>
    class AssetHandler<Mesh>
    {
      public:
        static Ref<Asset> Create() { return CreateRef<Mesh>(); }

        static Ref<Asset> Load(const std::filesystem::path& path, DataObject& file, DataObject& metafile) { return Mesh::LoadMesh(path); }

        static void Save(Ref<Asset>& asset, const std::filesystem::path& path, DataObject& file, DataObject& metafile) {}
    };

    template<>
    class AssetHandler<Texture>
    {
      public:
        static Ref<Asset> Create() { return CreateRef<Mesh>(); }

        static Ref<Asset> Load(const std::filesystem::path& path, DataObject& file, DataObject& metafile) 
        { 
            TextureSettings settings = DefaultTextureSettings;
            settings.Serialize(metafile);
            return TextureManager::LoadTexture(path, settings); 
        }

        static void Save(Ref<Asset>& asset, const std::filesystem::path& path, DataObject& file, DataObject& metafile) 
        {
            auto tex = std::static_pointer_cast<Texture>(asset);
            tex->GetSettings().Serialize(metafile);
        }
    };
} // namespace EVA
