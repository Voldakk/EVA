#pragma once

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "FileSystem.hpp"

namespace nlohmann
{
    template<>
    struct adl_serializer<std::filesystem::path>
    {
        static void to_json(json& j, const std::filesystem::path& path)
        { 
            j = EVA::FileSystem::ToString(path);
        }

        static void from_json(const json& j, std::filesystem::path& path)
        { 
            path = j.get<std::string>();
        }
    };
} // namespace nlohmann

namespace glm
{
    inline void to_json(json& j, const glm::vec2& v) { j = {v.x, v.y}; }

    inline void from_json(const json& j, glm::vec2& v)
    {
        j[0].get_to(v.x);
        j[1].get_to(v.y);
    }

    inline void to_json(json& j, const glm::vec3& v) { j = {v.x, v.y, v.z}; }

    inline void from_json(const json& j, glm::vec3& v)
    {
        j[0].get_to(v.x);
        j[1].get_to(v.y);
        j[2].get_to(v.z);
    }

    inline void to_json(json& j, const glm::vec4& v) { j = {v.x, v.y, v.z, v.w}; }

    inline void from_json(const json& j, glm::vec4& v)
    {
        j[0].get_to(v.x);
        j[1].get_to(v.y);
        j[2].get_to(v.z);
        j[3].get_to(v.w);
    }

    inline void to_json(json& j, const glm::ivec2& v) { j = {v.x, v.y}; }

    inline void from_json(const json& j, glm::ivec2& v)
    {
        j[0].get_to(v.x);
        j[1].get_to(v.y);
    }

    inline void to_json(json& j, const glm::ivec3& v) { j = {v.x, v.y, v.z}; }

    inline void from_json(const json& j, glm::ivec3& v)
    {
        j[0].get_to(v.x);
        j[1].get_to(v.y);
        j[2].get_to(v.z);
    }

    inline void to_json(json& j, const glm::ivec4& v) { j = {v.x, v.y, v.z, v.w}; }

    inline void from_json(const json& j, glm::ivec4& v)
    {
        j[0].get_to(v.x);
        j[1].get_to(v.y);
        j[2].get_to(v.z);
        j[3].get_to(v.w);
    }
} // namespace glm
