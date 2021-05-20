#pragma once

#include "Json.hpp"
#include "Guid.hpp"
#include "Asset.hpp"
#include "ISerializeable.hpp"
#include "DataObject.hpp"
#include "FileSystem.hpp"

namespace EVA
{
    constexpr int JSON_INDENT_WIDTH = 4;

    template<typename T>
    class AssetHandler
    {
      public:
        static Ref<Asset> Create() 
        { 
            auto asset = CreateRef<T>(); 
            return asset;
        }

        static Ref<Asset> Load(const Path& path, DataObject& file, DataObject& metafile) 
        { 
            auto asset = CreateRef<T>();
            asset->Serialize(file);
            return asset; 
        }
        static void Save(Ref<Asset>& asset, const Path& path, DataObject& file, DataObject& metafile) 
        {
            asset->Serialize(file);
        }
    };

    class AssetManager
    {
        struct FileInfo
        {
            bool isJson;
            Ref<Asset> (*create)();
            Ref<Asset> (*load)(const Path&, DataObject&, DataObject&);
            void (*save)(Ref<Asset>&, const Path&, DataObject&, DataObject&);
        };
        inline static std::unordered_map<std::string, FileInfo> s_FileTypes;
        inline static std::unordered_map<std::string, Ref<Asset>> s_Assets;

        inline static Path s_SharedPtrAssetPath = Path(".shared");
        inline static std::unordered_map<Ref<ISerializeable>, Guid> s_SharedPtrToIdMap;
        inline static std::map<Guid, Ref<ISerializeable>> s_SharedIdToPtrMap;
        inline static std::map<Guid, json> s_SharedIdToJson;

      public:
        AssetManager() = delete;

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static Ref<T> Create(const Path& path);
        static Ref<Asset> CreateAsset(const Path& path);

        static bool DeleteAsset(const Ref<Asset>& asset);
        static bool DeleteAsset(const Path& path);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static Ref<T> Load(const Path& path);
        static Ref<Asset> LoadAsset(const Path& path);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static Ref<T> Load(const Guid& guid);
        static Ref<Asset> LoadAsset(const Guid& guid);

        static bool Save(Ref<Asset>& asset);
        static bool Save(Ref<Asset>& asset, const Path& path);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static bool Save(Ref<T>& asset);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static bool Save(Ref<T>& asset, const Path& path);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static void Register(const Path& fileType, bool isJson);

        static Guid AddShared(const Ref<ISerializeable>& value);

        static bool SaveShared();
        static bool SaveShared(const Path& path);
        static bool LoadShared();
        static bool LoadShared(const Path& path);

        template<typename T, typename = std::enable_if_t<std::is_base_of<ISerializeable, T>::value>>
        static Ref<T> GetShared(const Guid guid);

        static void ClearAssets();
        static void ClearRegistered();
        static void ClearAll();

      private:
        static bool GetFileInfo(FileInfo& info, const Path& extension);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static Ref<Asset> CreateAssetT();

        static Ref<ISerializeable> GetExistingShared(const Guid guid);
        static Ref<ISerializeable> GetSharedFromJson(const Guid guid, Ref<ISerializeable>& value);

        static json ReadFile(const Path& path);
        static bool WriteFile(const Path& path, const json& data);
    };

    template<typename T, typename>
    inline Ref<T> AssetManager::Create(const Path& path)
    {
        return std::static_pointer_cast<T>(CreateAsset(path));
    }

    template<typename T, typename>
    inline Ref<T> AssetManager::Load(const Path& path)
    {
        return std::static_pointer_cast<T>(LoadAsset(path));
    }

    template<typename T, typename>
    inline Ref<T> AssetManager::Load(const Guid& guid)
    {
        return std::static_pointer_cast<T>(LoadAsset(guid));
    }

    template<typename T, typename>
    inline bool AssetManager::Save(Ref<T>& asset)
    {
        return Save(std::static_pointer_cast<Asset>(asset));
    }

    template<typename T, typename>
    inline bool AssetManager::Save(Ref<T>& asset, const Path& path)
    {
        return Save(std::static_pointer_cast<Asset>(asset), path);
    }

    template<typename T, typename>
    inline void AssetManager::Register(const Path& extension, bool isJson)
    {
        const auto extensionString = FileSystem::ToString(extension);

        auto it = s_FileTypes.find(extensionString);
        EVA_INTERNAL_ASSERT(it == s_FileTypes.end(), "Duplicate extension: {}", extensionString);

        s_FileTypes[extensionString] = {isJson, &AssetHandler<T>::Create, &AssetHandler<T>::Load, &AssetHandler<T>::Save};
    }

    template<typename T, typename>
    inline Ref<T> AssetManager::GetShared(const Guid guid)
    {
        Ref<T> value = std::static_pointer_cast<T>(GetExistingShared(guid));
        if (value == nullptr)
        {
            value = std::make_shared<T>();
            value = std::static_pointer_cast<T>(GetSharedFromJson(guid, std::static_pointer_cast<ISerializeable>(value)));
        }
        return value;
    }

    template<typename T, typename>
    inline Ref<Asset> AssetManager::CreateAssetT()
    {
        return AssetHandler<T>::Load();
    }
} // namespace EVA
