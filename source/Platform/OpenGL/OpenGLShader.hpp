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

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual const std::string& GetName() const override { return m_Name; };

        virtual void SetUniformInt(const std::string& name, const int value) override;
        virtual void SetUniformFloat(const std::string& name, const float value) override;
        virtual void SetUniformFloat2(const std::string& name, const glm::vec2& value) override;
        virtual void SetUniformFloat3(const std::string& name, const glm::vec3& value) override;
        virtual void SetUniformFloat4(const std::string& name, const glm::vec4& value) override;

        virtual void SetUniformMat3(const std::string& name, const glm::mat3& matrix) override;
        virtual void SetUniformMat4(const std::string& name, const glm::mat4& matrix) override;

        virtual void BindTexture(const std::string& name, const Ref<Texture>& texture) override;
        virtual void BindImageTexture(const std::string& name, const Ref<Texture>& texture) override;

        virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) override;
        virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ, uint32_t groupSizeX,
                                     uint32_t groupSizeY, uint32_t groupSizeZ) override;

        virtual void ResetTextureUnit() override { m_TextureUnit = 0; }

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
