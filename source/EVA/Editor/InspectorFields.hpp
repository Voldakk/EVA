#pragma once

#include <charconv>
#include <cstring>
#include <string.h>
#include <type_traits>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "EVA/Assets/FileSystem.hpp"
#include "EVA/Assets/ISerializeable.hpp"
#include "EVA/Assets/Asset.hpp"
#include <EVA/Renderer/Texture.hpp>

namespace EVA
{
    class InspectorFields
    {
      public:
        inline static const int STRING_LENGTH = 10000;
        inline static char* C_STRING          = new char[STRING_LENGTH];

        inline static const float INDENT = 10.0f;

        static char* GetCString(const std::string& value)
        {
            strcpy(C_STRING, value.c_str());
            return C_STRING;
        }

        inline static uint32_t s_Counter = 0;
        static uint32_t Count() { return s_Counter++; }
        static void ResetCounter() { s_Counter = 0; }


        static bool EnterInt(const char* name, int& value)
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
            return ImGui::InputScalar(name, type, &value);
        }

        static bool Serializeable(const char* name, ISerializeable& value);
        static bool Serializeable(const char* name, Ref<ISerializeable>& value);
        static bool AssetPath(const char* name, Ref<Asset>& value);

        inline static bool Default(const char* name, bool& value) { return ImGui::Checkbox(name, &value); }
        inline static bool Default(const char* name, int& value) { return ImGui::InputInt(name, &value); }
        inline static bool Default(const char* name, float& value) { return ImGui::DragFloat(name, &value, 0.01f, 0.0f, 0.0f, "%.5f"); }
        inline static bool Default(const char* name, std::string& value) { return String(name, value); }

        inline static bool Default(const char* name, glm::vec2& value)
        {
            return ImGui::DragFloat2(name, glm::value_ptr(value), 0.01f, 0.0f, 0.0f, "%.5f");
        }
        inline static bool Default(const char* name, glm::vec3& value)
        {
            return ImGui::DragFloat3(name, glm::value_ptr(value), 0.01f, 0.0f, 0.0f, "%.5f");
        }
        inline static bool Default(const char* name, glm::vec4& value)
        {
            return ImGui::DragFloat4(name, glm::value_ptr(value), 0.01f, 0.0f, 0.0f, "%.5f");
        }

        inline static bool Default(const char* name, glm::ivec2& value) { return ImGui::InputInt2(name, glm::value_ptr(value)); }
        inline static bool Default(const char* name, glm::ivec3& value) { return ImGui::InputInt3(name, glm::value_ptr(value)); }
        inline static bool Default(const char* name, glm::ivec4& value) { return ImGui::InputInt4(name, glm::value_ptr(value)); }

        inline static bool Default(const char* name, std::filesystem::path& value) { return Path(name, value); }

        static bool Default(const char* name, ISerializeable& value);


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

        template<typename T, typename U>
        inline static bool Default(const char* name, std::pair<T, U>& value)
        {
            ImGui::Text("%s", name);
            ImGui::PushID(&value);
            ImGui::Indent(INDENT);

            bool changed = false;
            changed |= Default("0", std::get<0>(value));
            changed |= Default("1", std::get<1>(value));

            ImGui::Unindent(INDENT);
            ImGui::PopID();

            return changed;
        }

        template<typename... T>
        inline static bool Default(const char* name, std::tuple<T...>& value)
        {
            ImGui::Text("%s", name);
            ImGui::PushID(&value);
            ImGui::Indent(INDENT);

            ResetCounter();
            bool changed = false;

            std::apply([&](auto&... x) { changed = (... | Default(std::to_string(Count()).c_str(), x)); }, value);

            ImGui::Unindent(INDENT);
            ImGui::PopID();

            return changed;
        }

        // Ref<T>
        template<typename T>
        static typename std::enable_if<!std::is_base_of<Asset, T>::value, bool>::type Default(const char* name, Ref<T>& value)
        {
            return Default(name, *value);
        }

        // Ref<Asset>
        template<typename T>
        static typename std::enable_if<std::is_base_of<Asset, T>::value, bool>::type Default(const char* name, Ref<T>& value)
        {
            auto ref     = std::static_pointer_cast<Asset>(value);
            bool changed = AssetPath(name, ref);
            value        = std::static_pointer_cast<T>(ref);
            return changed;
        }

        static bool Line(const char* name, glm::vec2* points, size_t count, glm::vec2 min, glm::vec2 max, bool loop, Ref<Texture> background = nullptr)
        {
            const ImVec2 margin(5, 5);
            const float handleRad = 10.0f;

            bool changed = false;
            ImGui::PushID(name);

            ImGui::Text(name);

            const ImVec2 pos = ImGui::GetCursorScreenPos();
            const ImVec2 region = ImGui::GetContentRegionAvail();
            const float size = glm::min(region.x, region.y);

            const ImRect window(pos, pos + ImVec2(size, size));
            const ImRect canvas(pos + margin, pos + ImVec2(size, size) - margin);

            ImGui::Dummy(window.GetSize());

            ImDrawList* pDrawList = ImGui::GetWindowDrawList();

            if (background) 
            { 
                pDrawList->AddImage((ImTextureID)background->GetRendererId(), canvas.Min, canvas.Max);
            }

            const auto transformPoint = [&](glm::vec2 p)
            {
                glm::vec2 t = (p - min) / (max - min);
                float x     = canvas.Min.x + t.x * canvas.GetWidth();
                float y     = canvas.Max.y - t.y * canvas.GetHeight();
                return ImVec2(x, y);
            };

            pDrawList->PathClear();
            for (size_t i = 0; i < count; i++) 
            {
                pDrawList->PathLineTo(transformPoint(points[i]));
            }
            if (loop && count > 0)
            {
                pDrawList->PathLineTo(transformPoint(points[0]));
            }

            pDrawList->PathStroke(IM_COL32(255, 255, 255, 255));
            
            bool hovered;
            bool held;
            for (size_t i = 0; i < count; i++)
            {
                const ImVec2 p = transformPoint(points[i]);

                const ImRect handleRect(ImVec2(p.x - handleRad, p.y - handleRad), ImVec2(p.x + handleRad, p.y + handleRad)); 
                ImGui::ButtonBehavior(handleRect, ImGui::GetID((void*)&points[i]), &hovered, &held);

                const uint32_t c = hovered ? 255 : 128;
                pDrawList->AddCircleFilled(p, handleRad, IM_COL32(c, c, c, 255), 16);

                const bool dragged = hovered && held;
                if (dragged) 
                { 
                    ImVec2 delta = (ImGui::GetMousePos() - p) / canvas.GetSize();
                    points[i] += glm::vec2(delta.x, -delta.y) * (max - min);
                    points[i] = glm::clamp(points[i], min, max);

                    changed = true;
                }
            }

            ImGui::PopID();
            return changed;
        }
    };

} // namespace EVA
