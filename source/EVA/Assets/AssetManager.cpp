#include "AssetManager.hpp"

#include <fstream>

#include "Asset.hpp"
#include "ISerializeable.hpp"
#include "FileSystem.hpp"

namespace EVA
{
	Ref<Asset> AssetManager::CreateAsset(const std::filesystem::path& path)
	{
		EVA_INTERNAL_TRACE("{}", FileSystem::ToString(path));
		const auto info = GetFileInfo(path.extension());
		EVA_INTERNAL_ASSERT(!info.useMetaFile, "Unable to create binary asset");
		auto asset = info.createInstance();
		Save(asset, path);
		return asset;
	}

	bool AssetManager::DeleteAsset(const Ref<Asset>& asset)
	{
		return DeleteAsset(asset->m_Path);
	}

	bool AssetManager::DeleteAsset(const std::filesystem::path& path)
	{
        const auto pathString = FileSystem::ToString(path);
        EVA_INTERNAL_TRACE("{}", pathString);

		auto it = m_Assets.find(pathString);
		if (it == m_Assets.end())
			return false;

		m_Assets.erase(pathString);
		return std::filesystem::remove(path);
	}

	Ref<Asset> AssetManager::LoadAsset(const std::filesystem::path& path)
	{
        EVA_INTERNAL_TRACE("{}", FileSystem::ToString(path));
		const auto info = GetFileInfo(path.extension());
		auto asset = info.createInstance();
		asset->m_Path = path;
		return LoadAsset(asset, path);
	}

	Ref<Asset> AssetManager::LoadAsset(Ref<Asset>& asset)
	{
        EVA_INTERNAL_TRACE("{}", FileSystem::ToString(asset->GetPath()));
		return LoadAsset(asset, asset->m_Path);
	}

	Ref<Asset> AssetManager::LoadAsset(Ref<Asset>& asset, const std::filesystem::path& path)
	{
        const auto pathString = FileSystem::ToString(path);
        EVA_INTERNAL_TRACE("{}, {}", FileSystem::ToString(asset->GetPath()), pathString);

		auto existing = m_Assets.find(pathString);
		if (existing != m_Assets.end())
			return existing->second;

		const auto info = GetFileInfo(path.extension());

		auto loadPath = path;
		if (info.useMetaFile)
		{
			loadPath += ".meta";
			if (!std::filesystem::exists(loadPath))
				Save(asset, path);
		}

		std::ifstream is;
		is.open(loadPath);
		if (is.fail())
		{
            EVA_INTERNAL_ERROR("Failed to open file: {}", FileSystem::ToString(loadPath));
			return nullptr;
		}

		json j;
		is >> j;
		is.close();

		DataObject data(DataMode::Load, j);

		asset->m_Path = path;
		asset->Serialize(data);

		m_Assets[pathString] = asset;

		return asset;
	}

	bool AssetManager::Save(Ref<Asset>& asset)
	{
        EVA_INTERNAL_TRACE("{}", FileSystem::ToString(asset->GetPath()));
		return Save(asset, asset->m_Path);
	}

	bool AssetManager::Save(Ref<Asset>& asset, const std::filesystem::path& newPath)
	{
        const auto assetPathString = FileSystem::ToString(asset->GetPath());
        const auto newPathString   = FileSystem::ToString(newPath);

        EVA_INTERNAL_TRACE("Saving {} as {}", assetPathString, newPathString);

		const auto info = GetFileInfo(newPath.extension());

		json j;
        DataObject data(DataMode::Save, j);
		asset->Serialize(data);

		std::filesystem::create_directories(newPath.parent_path());

		auto savePath = newPath;
		if (info.useMetaFile)
			savePath += ".meta";

		std::ofstream os;
		os.open(savePath);
		if (os.fail())
		{
            EVA_INTERNAL_ERROR("Failed to open file: {}", FileSystem::ToString(savePath));
			return false;
		}

		os << j.dump(JSON_INDENT_WIDTH);
		os.close();

		if (newPath != asset->m_Path)
		{
            m_Assets.erase(assetPathString);
            asset->m_Path           = newPath;
            m_Assets[newPathString] = asset;
		}

		return true;
	}

	uuid AssetManager::AddShared(const Ref<ISerializeable>& value)
	{
		auto it = m_SharedPtrToIdMap.find(value);
		if (it == m_SharedPtrToIdMap.end())
		{
			auto id = NewUUID();
			m_SharedPtrToIdMap[value] = id;
			m_SharedIdToPtrMap[id] = value;
			return id;
		}
		return it->second;
	}

	bool AssetManager::SaveShared()
	{
		return SaveShared(m_SharedPtrAssetPath);
	}

	bool AssetManager::SaveShared(const std::filesystem::path& path)
	{
        EVA_INTERNAL_TRACE("{}", FileSystem::ToString(path));

		json j;
		std::vector<uuid> deleteIds;
		for (auto& [key, value] : m_SharedIdToPtrMap)
		{
			if (value.use_count() == 2)
			{
				deleteIds.push_back(key);
				continue;
			}
			json v;
            DataObject d = DataObject(DataMode::Save, v);
			value->Serialize(d);
			j.push_back({ key, v });
		}

		std::ofstream os;
		os.open(path);
		if (os.fail())
		{
            EVA_INTERNAL_ERROR("Failed to open file: {}", FileSystem::ToString(path));
			return false;
		}

		os << j.dump(JSON_INDENT_WIDTH);
		os.close();

		for (auto& id : deleteIds)
		{
			auto value = m_SharedIdToPtrMap[id];
			m_SharedIdToPtrMap.erase(id);
			m_SharedPtrToIdMap.erase(value);
		}
		return true;
	}

	bool AssetManager::LoadShared()
	{
		return LoadShared(m_SharedPtrAssetPath);
	}

	bool AssetManager::LoadShared(const std::filesystem::path& path)
	{
        EVA_INTERNAL_TRACE("{}", FileSystem::ToString(path));

		std::ifstream is;
		is.open(path);
		if (is.fail())
		{
            EVA_INTERNAL_ERROR("Failed to open file: {}", FileSystem::ToString(path));
			return false;
		}

		json j;
		is >> j;
		is.close();
		for (auto& i : j)
		{
			auto id = i[0].get<uuid>();
			m_SharedIdToJson[id] = i[1];
		}

		return true;
	}

	void AssetManager::ClearAssets()
	{
		m_Assets.clear();

		m_SharedPtrToIdMap.clear();
		m_SharedIdToPtrMap.clear();
		m_SharedIdToJson.clear();
	}

	void AssetManager::ClearRegistered()
	{
		m_FileTypes.clear();
	}

	void AssetManager::ClearAll()
	{
		ClearAssets();
		ClearRegistered();
	}

	const AssetManager::FileInfo& AssetManager::GetFileInfo(const std::filesystem::path& extension)
	{
        const auto extensionString = FileSystem::ToString(extension);
        EVA_INTERNAL_TRACE("{}", extensionString);

		auto info = m_FileTypes.find(extensionString);
        EVA_INTERNAL_ASSERT(info != m_FileTypes.end(), "Unknown extension: {}", extensionString);
		return info->second;
	}

	Ref<ISerializeable> AssetManager::GetExistingShared(const uuid id)
	{
		EVA_INTERNAL_TRACE("{}", id.str().c_str());

		auto it = m_SharedIdToPtrMap.find(id);
		if (it != m_SharedIdToPtrMap.end())
			return it->second;
		return nullptr;
	}

	Ref<ISerializeable> AssetManager::GetSharedFromJson(const uuid id, Ref<ISerializeable>& value)
	{
		EVA_INTERNAL_TRACE("{}", id.str().c_str());

		auto it = m_SharedIdToJson.find(id);
		if (it != m_SharedIdToJson.end())
		{
            DataObject d = DataObject(DataMode::Load, it->second);
			value->Serialize(d);

			m_SharedIdToPtrMap[id] = value;
			m_SharedPtrToIdMap[value] = id;
			m_SharedIdToJson.erase(id);

			return value;
		}
		return nullptr;
	}
}
