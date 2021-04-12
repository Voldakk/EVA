#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "EVA/Renderer/Shader.hpp"
#include "EVA/Renderer/Texture.hpp"

namespace EVA
{
    class OpenGLShader : public Shader
    {
      public:
        explicit OpenGLShader(const std::string& filepath);
        explicit OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
        virtual ~OpenGLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual const std::string& GetName() const override { return m_Name; };

        void SetUniformInt(const std::string& name, const int value);
        void SetUniformFloat(const std::string& name, const float value);
        void SetUniformFloat2(const std::string& name, const glm::vec2& value);
        void SetUniformFloat3(const std::string& name, const glm::vec3& value);
        void SetUniformFloat4(const std::string& name, const glm::vec4& value);

        void SetUniformMat3(const std::string& name, const glm::mat3& matrix);
        void SetUniformMat4(const std::string& name, const glm::mat4& matrix);

        void BindTexture(const std::string& name, Ref<Texture> texture);
        void BindImageTexture(const std::string& name, Ref<Texture> texture);

        void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);
        void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);

        void ResetTextureUnit() { m_TextureUnit = 0; }

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
