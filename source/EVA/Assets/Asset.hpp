#pragma once

namespace EVA
{
    class DataObject;

    class Asset
	{
		friend class AssetManager;
		std::filesystem::path m_Path;

	public:

		Asset() = default;
		virtual ~Asset() = default;
        virtual void Serialize(DataObject& data);
		std::filesystem::path GetPath() { return m_Path; }

		bool operator==(const Asset& other) const 
		{ 
			return m_Path == other.m_Path;
		}
	};
}
