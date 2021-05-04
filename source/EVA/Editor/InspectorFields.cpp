#include "InspectorFields.hpp"
#include "EVA/Assets/ISerializeable.hpp"

namespace EVA
{
    bool InspectorFields::Serializeable(const char* name, Ref<ISerializeable>& value)
    {
        bool changed = false;
        if (ImGui::CollapsingHeader(name))
        {
            ImGui::Indent(INDENT);

            DataObject data;
            data.mode = DataObject::DataMode::Inspector;
            value->Serialize(data);

            ImGui::Unindent(INDENT);
        }
        return changed;
    }

    bool InspectorFields::Default(const char* name, Ref<ISerializeable>& value) { return Serializeable(name, value); }

    bool InspectorFields::Serializeable(const char* name, ISerializeable& value)
    {
        bool changed = false;
        if (ImGui::CollapsingHeader(name))
        {
            ImGui::Indent(INDENT);

            DataObject data;
            data.mode = DataObject::DataMode::Inspector;
            value.Serialize(data);

            ImGui::Unindent(INDENT);
        }
        return changed;
    }

    bool InspectorFields::Default(const char* name, ISerializeable& value) { return Serializeable(name, value); }
}
