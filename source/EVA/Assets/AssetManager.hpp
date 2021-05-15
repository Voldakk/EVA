#pragma once

#include "Json.hpp"
#include "UUID.hpp"
#include "Asset.hpp"
#include "ISerializeable.hpp"
#include "DataObject.hpp"

namespace EVA
{
    constexpr int JSON_INDENT_WIDTH = 4;

    class AssetManager
    {
        struct FileInfo
        {
            bool useMetaFile;
            Ref<Asset> (*createInstance)();
        };
        inline static std::unordered_map<std::string, FileInfo> m_FileTypes;
        inline static std::unordered_map<std::string, Ref<Asset>> m_Assets;

        inline static std::filesystem::path m_SharedPtrAssetPath = "./assets/.shared";
        inline static std::unordered_map<Ref<ISerializeable>, uuid> m_SharedPtrToIdMap;
        inline static std::map<uuid, Ref<ISerializeable>> m_SharedIdToPtrMap;
        inline static std::map<uuid, json> m_SharedIdToJson;

      public:
        AssetManager() = delete;

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static Ref<T> Create(const std::filesystem::path& path);
        static Ref<Asset> CreateAsset(const std::filesystem::path& path);

        static bool DeleteAsset(const Ref<Asset>& asset);
        static bool DeleteAsset(const std::filesystem::path& path);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static Ref<T> Load(const std::filesystem::path& path);
        static Ref<Asset> LoadAsset(const std::filesystem::path& path);
        static Ref<Asset> LoadAsset(Ref<Asset>& asset);
        static Ref<Asset> LoadAsset(Ref<Asset>& asset, const std::filesystem::path& path);

        static bool Save(Ref<Asset>& asset);
        static bool Save(Ref<Asset>& asset, const std::filesystem::path& path);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static bool Save(Ref<T>& asset);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static bool Save(Ref<T>& asset, const std::filesystem::path& path);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static void Register(const std::filesystem::path& fileType, bool useMetaFile);

        static uuid AddShared(const Ref<ISerializeable>& value);

        static bool SaveShared();
        static bool SaveShared(const std::filesystem::path& path);
        static bool LoadShared();
        static bool LoadShared(const std::filesystem::path& path);

        template<typename T, typename = std::enable_if_t<std::is_base_of<ISerializeable, T>::value>>
        static Ref<T> GetShared(const uuid id);

        static void ClearAssets();
        static void ClearRegistered();
        static void ClearAll();

      private:
        static const FileInfo& GetFileInfo(const std::filesystem::path& extension);

        template<typename T, typename = std::enable_if_t<std::is_base_of<Asset, T>::value>>
        static Ref<Asset> CreateAssetT();

        static Ref<ISerializeable> GetExistingShared(const uuid id);
        static Ref<ISerializeable> GetSharedFromJson(const uuid id, Ref<ISerializeable>& value);
    };

    template<typename T, typename>
    inline Ref<T> AssetManager::Create(const std::filesystem::path& path)
    {
        return std::static_pointer_cast<T>(CreateAsset(path));
    }

    template<typename T, typename>
    inline Ref<T> AssetManager::Load(const std::filesystem::path& path)
    {
        return std::static_pointer_cast<T>(LoadAsset(path));
    }

    template<typename T, typename>
    inline bool AssetManager::Save(Ref<T>& asset)
    {
        return Save(std::static_pointer_cast<Asset>(asset));
    }

    template<typename T, typename>
    inline bool AssetManager::Save(Ref<T>& asset, const std::filesystem::path& path)
    {
        return Save(std::static_pointer_cast<Asset>(asset), path);
    }

    template<typename T, typename>
    inline void AssetManager::Register(const std::filesystem::path& extension, bool useMetaFile)
    {
        const auto extensionString = FileSystem::ToString(extension);
        EVA_INTERNAL_TRACE("{}, useMetaFile={}", extensionString, useMetaFile ? "true" : "false");


        auto it = m_FileTypes.find(extensionString);
        if (it != m_FileTypes.end()) { EVA_INTERNAL_ERROR("Duplicate extension: {}", extensionString); }

        m_FileTypes[extensionString] = {useMetaFile, &CreateAssetT<T>};
    }

    template<typename T, typename>
    inline Ref<T> AssetManager::GetShared(const uuid id)
    {
        Ref<T> value = std::static_pointer_cast<T>(GetExistingShared(id));
        if (value == nullptr)
        {
            value = std::make_shared<T>();
            value = std::static_pointer_cast<T>(GetSharedFromJson(id, std::static_pointer_cast<ISerializeable>(value)));
        }
        return value;
    }

    template<typename T, typename>
    inline Ref<Asset> AssetManager::CreateAssetT()
    {
        return std::make_shared<T>();
    }
} // namespace EVA
