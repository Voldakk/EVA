#pragma once

#include <glad/glad.h>

#include "EVA/Renderer/Shader.hpp"

namespace EVA
{
    class OpenGLShader : public Shader
    {
      public:
        explicit OpenGLShader(const std::string& filepath);
        explicit OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
        virtual ~OpenGLShader();

        void Bind() const override;
        void Unbind() const override;

        const std::string& GetName() const override { return m_Name; };

        void SetUniformBool(const std::string& name, const bool value) override;
        void SetUniformInt(const std::string& name, const int value) override;
        void SetUniformInt2(const std::string& name, const glm::ivec2& value) override;
        void SetUniformInt3(const std::string& name, const glm::ivec3& value) override;
        void SetUniformInt4(const std::string& name, const glm::ivec4& value) override;
        void SetUniformFloat(const std::string& name, const float value) override;
        void SetUniformFloat2(const std::string& name, const glm::vec2& value) override;
        void SetUniformFloat3(const std::string& name, const glm::vec3& value) override;
        void SetUniformFloat4(const std::string& name, const glm::vec4& value) override;

        void SetUniformMat3(const std::string& name, const glm::mat3& matrix) override;
        void SetUniformMat4(const std::string& name, const glm::mat4& matrix) override;

        void BindTexture(const std::string& name, const Ref<Texture>& texture) override;
        void BindTexture(const std::string& name, const TextureTarget target, const uint32_t rendererId) override;
        void BindImageTexture(const uint32_t location, const Ref<Texture>& texture, const TextureAccess access) override;

        void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) override;
        void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ, uint32_t groupSizeX, uint32_t groupSizeY,
                             uint32_t groupSizeZ) override;

        void ResetTextureUnit() override { m_TextureUnit = 0; }

      private:
        std::string ReadFile(const std::string& filepath);
        std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
        void Compile(const std::unordered_map<GLenum, std::string>& sources);

        GLint GetUniformLocation(const std::string& name);
        void ResetUniformLocations() { m_UniformLocationMap.clear(); }

        uint32_t m_RendererId;
        std::string m_Name;
        std::unordered_map<std::string, GLint> m_UniformLocationMap;
        uint32_t m_TextureUnit = 0;
    };
} // namespace EVA
