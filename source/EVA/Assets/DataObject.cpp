#include "DataObject.hpp"
#include "Asset.hpp"
#include "AssetManager.hpp"

namespace EVA
{
    DataObject::DataObject(DataMode mode) : m_Mode(mode), m_Json(m_InternalJson) {}
    DataObject::DataObject(DataMode mode, json& json) : m_Mode(mode), m_Json(json) {}

    Ref<Asset> DataObject::GetAsset(const Guid& guid) const
    {
        return AssetManager::LoadAsset(guid);
    }
} // namespace EVA