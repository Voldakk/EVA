#pragma once

#include "Json.hpp"

namespace EVA
{
    class DataObject;
    class ISerializeable
    {
      public:
        virtual ~ISerializeable() = default;
        virtual void Serialize(DataObject& data);
        virtual std::string GetTypeId() const { return ""; }
    };
} // namespace EVA::AssetManagement


#include "EVA/Utility/ClassMap.hpp"

CREATE_CLASS_MAP(EVAISerializeable, EVA::ISerializeable)

// Macro for registering a component. Should be put inside the class declaration
#define REGISTER_SERIALIZABLE(TYPE) REGISTER_CLASS(EVAISerializeable, TYPE, #TYPE)
