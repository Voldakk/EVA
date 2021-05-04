#pragma once

#include <charconv>
#include <cstring>
#include <imgui.h>

#include "EVA/Assets/FileSystem.hpp"

namespace EVA
{
    class ISerializeable;

    class InspectorFields
    {
      public:
        inline static const int STRING_LENGTH = 10000;
        inline static char* C_STRING          = new char[STRING_LENGTH];

        inline static const float INDENT = 10.0f;

        static char* GetCString(const std::string& value)
        {
            strcpy_s(C_STRING, 10000, value.c_str());
            return C_STRING;
        }

        bool EnterInt(const char* name, int& value)
        {
            auto temp = value;
            if (ImGui::InputInt(name, &temp, 1, 5, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                value = temp;
                return true;
            }

            return false;
        }

        static bool String(const char* name, std::string& value)
        {
            auto string = GetCString(value);

            bool changed = ImGui::InputText(name, string, STRING_LENGTH);

            value = string;

            return changed;
        }

        static bool EnterString(const char* name, std::string& value)
        {
            auto string = GetCString(value);
            if (ImGui::InputText(name, string, STRING_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                value = string;
                return true;
            }

            return false;
        }

        static bool DragDropTargetString(const char* name, std::string& value, const char* payloadType)
        {
            auto string = GetCString(value);
            ImGui::BeginGroup();
            if (ImGui::InputText(name, string, STRING_LENGTH, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                value = string;
                ImGui::EndGroup();
                return true;
            }
            if (ImGui::BeginDragDropTarget())
            {
                const auto payload = ImGui::AcceptDragDropPayload(payloadType);
                if (payload)
                {
                    const char* path = (char*)payload->Data;
                    value            = path;

                    ImGui::EndDragDropTarget();
                    ImGui::EndGroup();
                    return true;
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::EndGroup();

            return false;
        }

        static bool Path(const char* name, std::filesystem::path& value)
        {
            auto pathString = FileSystem::ToString(value);
            bool changed    = DragDropTargetString(name, pathString, "file");
            value           = pathString;
            return changed;
        }

        template<typename T>
        static bool Vector(const char* name, std::vector<T>& value)
        {
            bool changed = false;
            if (ImGui::CollapsingHeader(name))
            {
                ImGui::Indent(INDENT);
                ImGui::PushID(&value);
                for (size_t i = 0; i < value.size(); i++)
                {
                    changed |= Default(std::to_string(i).c_str(), value[i]);
                }
                ImGui::PopID();
                ImGui::Unindent(INDENT);
            }
            return changed;
        }

        template<typename T>
        static bool Integral(const char* name, T& value) 
        { 
            static_assert(std::is_integral<T>::value, "Integral required.");
            constexpr size_t s = sizeof(T);
            ImGuiDataType type = 0;
            if constexpr (std::is_signed_v<T>)
            {
                if constexpr (s == 2) type = ImGuiDataType_S8;
                if constexpr (s == 4) type = ImGuiDataType_S16;
                if constexpr (s == 8) type = ImGuiDataType_S32;
                if constexpr (s == 16) type = ImGuiDataType_S64;
            }
            else
            {
                if constexpr (s == 2) type = ImGuiDataType_U8;
                if constexpr (s == 4) type = ImGuiDataType_U16;
                if constexpr (s == 8) type = ImGuiDataType_U32;
                if constexpr (s == 16) type = ImGuiDataType_U64;
            }
            return ImGui::InputScalar(name, type , &value); 
        }

        static bool Serializeable(const char* name, ISerializeable& value);
        static bool Serializeable(const char* name, Ref<ISerializeable>& value);

        inline static bool Default(const char* name, bool& value) { return ImGui::Checkbox(name, &value); }
        inline static bool Default(const char* name, int& value) { return ImGui::InputInt(name, &value); }
        inline static bool Default(const char* name, float& value) { return ImGui::DragFloat(name, &value, 0.01f, 0.0f, 0.0f, "%.5f"); }
        inline static bool Default(const char* name, std::string& value) { return String(name, value); }

        inline static bool Default(const char* name, glm::vec2& value) { return ImGui::DragFloat2(name, glm::value_ptr(value), 0.01f, 0.0f, 0.0f, "%.5f"); }
        inline static bool Default(const char* name, glm::vec3& value) { return ImGui::DragFloat3(name, glm::value_ptr(value), 0.01f, 0.0f, 0.0f, "%.5f"); }
        inline static bool Default(const char* name, glm::vec4& value) { return ImGui::DragFloat4(name, glm::value_ptr(value), 0.01f, 0.0f, 0.0f, "%.5f"); }

        inline static bool Default(const char* name, glm::ivec2& value) { return ImGui::InputInt2(name, glm::value_ptr(value)); }
        inline static bool Default(const char* name, glm::ivec3& value) { return ImGui::InputInt3(name, glm::value_ptr(value)); }
        inline static bool Default(const char* name, glm::ivec4& value) { return ImGui::InputInt4(name, glm::value_ptr(value)); }

        inline static bool Default(const char* name, std::filesystem::path& value) { return Path(name, value); }

        static bool Default(const char* name, ISerializeable& value);
        static bool Default(const char* name, Ref<ISerializeable>& value);

        template<typename T, typename Alloc>
        inline static bool Default(const char* name, std::vector<T, Alloc>& value)
        {
            return Vector(name, value);
        }

        template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
        inline static bool Default(const char* name, T& value)
        {
            return Integral(name, value);
        }
    };

} // namespace EVA
