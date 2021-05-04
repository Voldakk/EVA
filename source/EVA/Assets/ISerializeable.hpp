#pragma once

#include "EVA/Editor/InspectorFields.hpp"

namespace EVA
{
    class DataObject
    {
      public:
        enum class DataMode
        {
            Save,
            Load,
            Inspector
        };

        DataObject() = default;

        template<typename T>
        bool Serialize(const std::string& key, T& value);

        template<typename T>
        bool Serialize(const std::string& key, T& value, float min, float max);

        DataMode mode = DataMode::Inspector;
        bool changed  = false;
        
        inline bool Inspector() const { return mode == DataMode::Inspector; }
    };

    class ISerializeable
    {
      public:
        virtual void Serialize(DataObject& data) {}
    };

    template<typename T>
    bool DataObject::Serialize(const std::string& key, T& value)
    {
        switch (mode)
        {
            case DataMode::Save:
                // Set(key, value);
                return false;

            case DataMode::Load:
                // value   = Get(key, value);
                changed = true;
                return true;

            case DataMode::Inspector:
                bool c = InspectorFields::Default(key.c_str(), value);
                if (c) changed = true;
                return c;
        }
        EVA_INTERNAL_ASSERT(false, "Unknown DataMode");
        return false;
    }
} // namespace EVA