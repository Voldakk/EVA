#pragma once

#include <iostream>
#include <type_traits>

#include "Json.hpp"
#include "UUID.hpp"
#include "AssetManager.hpp"
#include "ISerializeable.hpp"
#include "EVA/Editor/InspectorFields.hpp"

namespace EVA
{
	class Asset;

	enum class DataMode
	{
		Save,
		Load,
        Inspector
	};

	class DataObject
	{
		template <class T> struct is_shared_ptr : std::false_type {};
		template <class T> struct is_shared_ptr<std::shared_ptr<T> > : std::true_type {};

		DataMode m_Mode;
		json& m_Json;
		json m_InternalJson;

	public:
        bool changed = false;

		DataObject(DataMode mode);
		DataObject(DataMode mode, json& json);
        ~DataObject() = default;

		inline bool Inspector() const { return m_Mode == DataMode::Inspector; }
        inline bool Load() const { return m_Mode == DataMode::Load; }
        inline bool Save() const { return m_Mode == DataMode::Save; }

		template <typename T>
		bool Serialize(const std::string& name, T& value)
		{
			switch (m_Mode)
			{
			case DataMode::Save:
				Set(name, value);
				return false;
			case DataMode::Load:
                value     = Get(name, value);
                changed = true;
				return true;
            case DataMode::Inspector:
            {
                bool c = InspectorFields::Default(name.c_str(), value);
                changed |= c;
                return c;
            }
			default:
				EVA_INTERNAL_ASSERT(false, "Invalid enum value");
			}
			return false;
		}

		// T
		template <typename T>
		typename std::enable_if<!is_shared_ptr<T>::value && !(std::is_base_of<ISerializeable, T>::value || std::is_base_of<Asset, T>::value)>::type
			Set(const std::string& key, T& value) const
		{
			m_Json[key] = value;
		}

		template <typename T>
		typename std::enable_if<!is_shared_ptr<T>::value && !(std::is_base_of<ISerializeable, T>::value || std::is_base_of<Asset, T>::value), T>::type
			Get(const std::string& key, T& defaultValue) const
		{
			if (m_Json.find(key) == m_Json.end())
				return defaultValue;

			return m_Json[key].get<T>();
		}


		// ISerializeable
		template <typename T>
		typename std::enable_if<std::is_base_of<ISerializeable, T>::value>::type
			Set(const std::string& key, T& value) const
		{
			json j;
            DataObject d = DataObject(m_Mode, j);
			value.Serialize(d);
			m_Json[key] = j;
		}

		template <typename T>
		typename std::enable_if<std::is_base_of<ISerializeable, T>::value, T>::type
			Get(const std::string& key, T& defaultValue) const
		{
			if (m_Json.find(key) == m_Json.end())
				return defaultValue;

			json j = m_Json[key];
            DataObject d = DataObject(m_Mode, j);
			T t;
			t.Serialize(d);
			return t;
		}


		// Asset
		/*template <typename T>
		typename std::enable_if<std::is_base_of<Asset, T>::value>::type
			Set(const std::string& key, T& value) const
		{
            m_Json[key] = value.GetPath();
		}

		template <typename T>
		typename std::enable_if<std::is_base_of<Asset, T>::value, T>::type
			Get(const std::string& key, T& defaultValue) const
		{
			if (m_Json.find(key) == m_Json.end())
				return defaultValue;

			std::filesystem::path = m_Json[key];
			return 0;
		}*/

		// Ref<T>
		template<typename T>
		typename std::enable_if<!(std::is_base_of<ISerializeable, T>::value || std::is_base_of<Asset, T>::value)>::type
			Set(const std::string& key, Ref<T>& value) const
		{
            static_assert(false, "Not supported");
		}

		template<typename T>
        typename std::enable_if<!(std::is_base_of<ISerializeable, T>::value || std::is_base_of<Asset, T>::value), Ref<T>>::type
          Get(const std::string& key, Ref<T>& defaultValue) const
		{
			static_assert(false, "Not supported");
			return defaultValue;
		}


		// Ref<ISerializeable>
		/*template <typename T>
		typename std::enable_if<std::is_base_of<ISerializeable, T>::value>::type Set(const std::string& key, Ref<T>& value) const
		{
			if (value != nullptr)
			{
				const auto id = AssetManager::AddShared(value);
				m_Json[key] = id;
			}
		}

		template <typename T>
        typename std::enable_if<std::is_base_of<ISerializeable, T>::value, Ref<T>>::type Get(const std::string& key, Ref<T>& defaultValue) const
		{
			if (m_Json.find(key) == m_Json.end())
				return defaultValue;

			const auto id = m_Json[key].get<uuid>();
			return AssetManager::GetShared<T>(id);
		}*/
        template <typename T>
        typename std::enable_if<std::is_base_of<ISerializeable, T>::value>::type Set(const std::string& key, Ref<T>& value) const
        {
            if (value != nullptr)
            { 
                DataObject d = DataObject(m_Mode);
                if (value->GetTypeId() != "") { d.Set("ISerializeableTypeId", value->GetTypeId()); }
                value->Serialize(d);
                m_Json[key] = d.m_Json;
            }
        }

        template <typename T>
        typename std::enable_if<std::is_base_of<ISerializeable, T>::value, Ref<T>>::type Get(const std::string& key, Ref<T>& defaultValue) const
        {
            if (m_Json.find(key) == m_Json.end())
                return defaultValue;

            DataObject d = DataObject(m_Mode, m_Json[key]);

			std::string ds = "";
            std::string type = d.Get<std::string>("ISerializeableTypeId", ds);
            Ref<ISerializeable> ref;
            if (type == ds) { ref = CreateRef<T>(); }
            else
            {
                ref = ClassMapEVAISerializeable::Create(type);
            }
            
            ref->Serialize(d);
            return std::static_pointer_cast<T>(ref);
        }


		// Ref<Asset>
		template <typename T>
		typename std::enable_if<std::is_base_of<Asset, T>::value>::type Set(const std::string& key, Ref<T>& value) const
		{
			if(value != nullptr)
				m_Json[key] = FileSystem::ToString(value->GetPath());
		}

		template <typename T>
        typename std::enable_if<std::is_base_of<Asset, T>::value, Ref<T>>::type Get(const std::string& key, Ref<T>& defaultValue) const
		{
			if (m_Json.find(key) == m_Json.end())
				return defaultValue;

			std::filesystem::path path = m_Json[key].get<std::string>();
			
			return AssetManager::Load<T>(path);
		}

		// std::vector<T>
        template<typename T, typename Alloc /*, typename = std::enable_if_t<std::is_base_of<ISerializeable, T>::value>*/>
        void Set(const std::string& key, std::vector<T, Alloc>& value)
        {
            json vec;
            for (auto& v : value)
            {
                DataObject d(DataMode::Save);
                d.Set("key", v);
                vec.push_back(d.m_Json["key"]);
            }
            m_Json[key] = vec;
        }

        template<typename T, typename Alloc>
        /*typename std::enable_if<std::is_base_of<ISerializeable, T>::value, std::vector<T, Alloc>>::type*/ std::vector<T, Alloc>
          Get(const std::string& key, std::vector<T, Alloc>& defaultValue)
        {
            if (m_Json.find(key) == m_Json.end()) return defaultValue;

            auto j = m_Json[key];
            std::vector<T, Alloc> vec(j.size());

            for (size_t i = 0; i < j.size(); i++)
            {
                DataObject d(DataMode::Load);
                d.m_Json["key"] = j[i];
                vec[i]          = d.Get("key", vec[i]);
            }

            return vec;
        }

		/*// std::vector<Ref<T>>
        template<typename T, typename Alloc, typename = std::enable_if_t<std::is_base_of<ISerializeable, T>::value>>
        void Set(const std::string& key, std::vector<Ref<T>, Alloc>& value)
        {
            json vec;
            for (auto v : value)
            {
                json element;
                DataObject d(DataMode::Save, element);

                v->Serialize(d);
                vec.push_back(element);
            }
            m_Json[key] = vec;
        }

        template<typename T, typename Alloc>
        typename std::enable_if<std::is_base_of<ISerializeable, T>::value, std::vector<Ref<T>, Alloc>>::type
          Get(const std::string& key, std::vector<Ref<T>, Alloc>& defaultValue)
        {
            if (m_Json.find(key) == m_Json.end()) return defaultValue;

            auto j = m_Json[key];
            std::vector<Ref<T>, Alloc> vec(j.size());

            for (size_t i = 0; i < j.size(); i++)
            {
                DataObject d(DataMode::Load, j[i]);
                vec[i]->Serialize(d);
            }

            return vec;
        }*/
	};
}
