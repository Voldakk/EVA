#pragma once

#include "Guid.hpp"

namespace EVA
{
    class DataObject;

    class Asset
    {
      protected:
        friend class AssetManager;
        template<typename T>
        friend class AssetHandler;
        Path m_Path;
        Guid m_Guid;

      public:
        Asset()          = default;
        virtual ~Asset() = default;
        virtual void Serialize(DataObject& data);
        Path GetPath() const { return m_Path; }
        Guid GetGuid() const { return m_Guid; }

        bool operator==(const Asset& other) const { return m_Path == other.m_Path; }
    };
} // namespace EVA
