#pragma once

#include "EVA/Renderer/Texture.hpp"

namespace EVA
{
    class Shader
    {
      public:
        virtual ~Shader() = default;

        virtual void Bind() const   = 0;
        virtual void Unbind() const = 0;

        virtual const std::string& GetName() const = 0;

        virtual void SetUniformBool(const std::string& name, const bool value)         = 0;
        virtual void SetUniformInt(const std::string& name, const int value)           = 0;
        virtual void SetUniformInt2(const std::string& name, const glm::ivec2& value)  = 0;
        virtual void SetUniformInt3(const std::string& name, const glm::ivec3& value)  = 0;
        virtual void SetUniformInt4(const std::string& name, const glm::ivec4& value)  = 0;
        virtual void SetUniformFloat(const std::string& name, const float value)       = 0;
        virtual void SetUniformFloat2(const std::string& name, const glm::vec2& value) = 0;
        virtual void SetUniformFloat3(const std::string& name, const glm::vec3& value) = 0;
        virtual void SetUniformFloat4(const std::string& name, const glm::vec4& value) = 0;

        virtual void SetUniformMat3(const std::string& name, const glm::mat3& matrix) = 0;
        virtual void SetUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;

        virtual void BindTexture(const std::string& name, const Ref<Texture>& texture)                                  = 0;
        virtual void BindTexture(const std::string& name, const TextureTarget target, const uint32_t rendererId)        = 0;
        virtual void BindImageTexture(const uint32_t location, const Ref<Texture>& texture, const TextureAccess access) = 0;

        virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) = 0;
        virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ, uint32_t groupSizeX,
                                     uint32_t groupSizeY, uint32_t groupSizeZ)                      = 0;

        virtual void ResetTextureUnit() = 0;

        static Ref<Shader> Create(const std::string& path);
        static Ref<Shader> Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
    };

    class ShaderLibrary
    {
        std::unordered_map<std::string, Ref<Shader>> m_Shaders;

      public:
        void Add(const Ref<Shader>& shader);
        void Add(const std::string& name, const Ref<Shader>& shader);
        Ref<Shader> Load(const std::string& filepath);
        Ref<Shader> Load(const std::string& name, const std::string& filepath);
        Ref<Shader> Get(const std::string& name);
        bool Exists(const std::string& name) const;
    };
} // namespace EVA
