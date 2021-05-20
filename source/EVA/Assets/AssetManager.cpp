#include "AssetManager.hpp"

#include <fstream>

#include "Asset.hpp"
#include "ISerializeable.hpp"
#include "FileSystem.hpp"

namespace EVA
{
    Ref<Asset> AssetManager::CreateAsset(const Path& path)
    {
        EVA_PROFILE_FUNCTION();
        FileInfo info;
        if (!GetFileInfo(info, path.extension())) { return nullptr; }
        auto asset = info.create();
        asset->m_Guid   = NewGuid();
        Save(asset, path);
        return asset;
    }

    bool AssetManager::DeleteAsset(const Ref<Asset>& asset) { return DeleteAsset(asset->m_Path); }

    bool AssetManager::DeleteAsset(const Path& path)
    {
        EVA_PROFILE_FUNCTION();
        const auto pathString = FileSystem::ToString(path);

        auto it = s_Assets.find(pathString);
        if (it == s_Assets.end()) return false;

        s_Assets.erase(pathString);

        auto metaPath = path;
        metaPath += ".meta";
        FileSystem::DeleteFile(metaPath);
        return FileSystem::DeleteFile(path);
    }

    Ref<Asset> AssetManager::LoadAsset(const Path& path)
    {
        EVA_PROFILE_FUNCTION();

        const auto pathString = FileSystem::ToString(path);

        auto existing = s_Assets.find(pathString);
        if (existing != s_Assets.end()) return existing->second;

        FileInfo info;
        if (!GetFileInfo(info, path.extension())) { return nullptr; }

        auto metaPath = path;
        metaPath += ".meta";

        // Create a .meta file is it does not exist
        if (!FileSystem::FileExists(metaPath))
        {
            json jsonNewMeta;
            jsonNewMeta["guid"] = NewGuid();

            bool res = WriteFile(metaPath, jsonNewMeta);
            if (!res) { return nullptr; }
        }

        // File
        json jsonFile;
        if (info.isJson)
        {
            jsonFile = ReadFile(path);
        }
        DataObject file(DataMode::Load, jsonFile);

        // Meta
        json jsonMeta = ReadFile(metaPath);
        DataObject meta(DataMode::Load, jsonMeta);

        auto asset = info.load(path, file, meta);
        asset->m_Path = path;
        s_Assets[pathString] = asset;

        Guid null;
        asset->m_Guid = meta.Get<Guid>("guid", null);
        if (asset->m_Guid == null)
        {
            EVA_INTERNAL_WARN("Guid not found for {}", FileSystem::ToString(path));
            asset->m_Guid = NewGuid();
        }

        return asset;
    }

    Ref<Asset> AssetManager::LoadAsset(const Guid& guid) 
    { 
        EVA_PROFILE_FUNCTION();

        //TODO
        return nullptr;
    }

    bool AssetManager::Save(Ref<Asset>& asset)
    {
        EVA_INTERNAL_ASSERT(!asset->GetPath().empty(), "Can't save an asset without a path");
        return Save(asset, asset->m_Path);
    }

    bool AssetManager::Save(Ref<Asset>& asset, const Path& path)
    {
        EVA_PROFILE_FUNCTION();

        const auto assetPathString = FileSystem::ToString(asset->GetPath());
        const auto pathString = FileSystem::ToString(path);

        FileInfo info;
        if (!GetFileInfo(info, path.extension())) { return false; }

        json jsonFile;
        DataObject file(DataMode::Save, jsonFile);
        json jsonMeta;
        DataObject meta(DataMode::Save, jsonMeta);

        info.save(asset, path, file, meta);

        // File
        if (info.isJson)
        {
            if (!WriteFile(path, jsonFile)) { return false; }
        }

        // Meta
        auto metaPath = path;
        metaPath += ".meta";
        if (!WriteFile(metaPath, jsonMeta)) { return false; }

        // Cache
        if (path != asset->m_Path)
        {
            s_Assets.erase(assetPathString);
            asset->m_Path        = path;
            s_Assets[pathString] = asset;
        }

        return true;
    }

    Guid AssetManager::AddShared(const Ref<ISerializeable>& value)
    {
        EVA_PROFILE_FUNCTION();

        auto it = s_SharedPtrToIdMap.find(value);
        if (it == s_SharedPtrToIdMap.end())
        {
            auto id                   = NewGuid();
            s_SharedPtrToIdMap[value] = id;
            s_SharedIdToPtrMap[id]    = value;
            return id;
        }
        return it->second;
    }

    bool AssetManager::SaveShared() { return SaveShared(s_SharedPtrAssetPath); }

    bool AssetManager::SaveShared(const Path& path)
    {
        EVA_PROFILE_FUNCTION();

        json j;
        std::vector<Guid> deleteIds;
        for (auto& [key, value] : s_SharedIdToPtrMap)
        {
            if (value.use_count() == 2)
            {
                deleteIds.push_back(key);
                continue;
            }
            json v;
            DataObject d = DataObject(DataMode::Save, v);
            value->Serialize(d);
            j.push_back({key, v});
        }

        if (!WriteFile(path, j)) { return false; }

        for (auto& id : deleteIds)
        {
            auto value = s_SharedIdToPtrMap[id];
            s_SharedIdToPtrMap.erase(id);
            s_SharedPtrToIdMap.erase(value);
        }
        return true;
    }

    bool AssetManager::LoadShared() { return LoadShared(s_SharedPtrAssetPath); }

    bool AssetManager::LoadShared(const Path& path)
    {
        EVA_PROFILE_FUNCTION();

        json j = ReadFile(path);

        for (auto& i : j)
        {
            auto id              = i[0].get<Guid>();
            s_SharedIdToJson[id] = i[1];
        }

        return true;
    }

    void AssetManager::ClearAssets()
    {
        EVA_PROFILE_FUNCTION();

        s_Assets.clear();
        s_SharedPtrToIdMap.clear();
        s_SharedIdToPtrMap.clear();
        s_SharedIdToJson.clear();
    }

    void AssetManager::ClearRegistered() { s_FileTypes.clear(); }

    void AssetManager::ClearAll()
    {
        ClearAssets();
        ClearRegistered();
    }

    bool AssetManager::GetFileInfo(FileInfo& info, const Path& extension)
    {
        EVA_PROFILE_FUNCTION();

        const auto extensionString = FileSystem::ToString(extension);
        
        auto it = s_FileTypes.find(extensionString);

        if (it != s_FileTypes.end()) {
            info = it->second;
            return true;
        }

        EVA_INTERNAL_WARN("Unknown extension: {}", extensionString);
        return false;
    }

    Ref<ISerializeable> AssetManager::GetExistingShared(const Guid guid)
    {
        EVA_PROFILE_FUNCTION();

        auto it = s_SharedIdToPtrMap.find(guid);
        if (it != s_SharedIdToPtrMap.end()) return it->second;
        return nullptr;
    }

    Ref<ISerializeable> AssetManager::GetSharedFromJson(const Guid guid, Ref<ISerializeable>& value)
    {
        EVA_PROFILE_FUNCTION();

        auto it = s_SharedIdToJson.find(guid);
        if (it != s_SharedIdToJson.end())
        {
            DataObject d = DataObject(DataMode::Load, it->second);
            value->Serialize(d);

            s_SharedIdToPtrMap[guid]  = value;
            s_SharedPtrToIdMap[value] = guid;
            s_SharedIdToJson.erase(guid);

            return value;
        }
        return nullptr;
    }

    json AssetManager::ReadFile(const Path& path) 
    {
        EVA_PROFILE_FUNCTION();

        json data;
        std::ifstream stream;
        if (!FileSystem::OpenFile(stream, path)) { return nullptr; }
        stream >> data;
        stream.close();

        return data;
    }

    bool AssetManager::WriteFile(const Path& path, const json& data) 
    {
        EVA_PROFILE_FUNCTION();

        FileSystem::CreateDirectories(path.parent_path());

        std::ofstream stream;
        if (!FileSystem::OpenFile(stream, path)) { return nullptr; }
        stream << data.dump(JSON_INDENT_WIDTH);
        stream.close();

        return true;
    }
} // namespace EVA
