#include "OpenGLShader.hpp"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include "OpenGLTexture.hpp"

namespace EVA
{
    static GLenum ShaderTypeFromString(const std::string& type)
    {
        if (type == "vertex") return GL_VERTEX_SHADER;
        if (type == "fragment") return GL_FRAGMENT_SHADER;
        if (type == "geometry") return GL_GEOMETRY_SHADER;
        if (type == "tess_control") return GL_TESS_CONTROL_SHADER;
        if (type == "tess_evaluation") return GL_TESS_EVALUATION_SHADER;
        if (type == "compute") return GL_COMPUTE_SHADER;

        EVA_INTERNAL_ASSERT(false, "Invalid shader type: '{}'", type);
        return 0;
    }

    OpenGLShader::OpenGLShader(const std::string& filepath)
    {
        EVA_PROFILE_FUNCTION();

        auto source  = ReadFile(filepath);
        auto sources = PreProcess(source);
        Compile(sources);

        auto lastSlash = filepath.find_last_of("/\\");
        lastSlash      = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        auto lastDot   = filepath.rfind('.');
        auto count     = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
        m_Name         = filepath.substr(lastSlash, count);
    }

    OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource) : m_Name(name)
    {
        EVA_PROFILE_FUNCTION();

        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER]   = vertexSource;
        sources[GL_FRAGMENT_SHADER] = fragmentSource;
        Compile(sources);
    }

    std::string OpenGLShader::ReadFile(const std::string& filepath)
    {
        EVA_PROFILE_FUNCTION();

        std::string result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
            in.close();
        }
        else
        {
            EVA_INTERNAL_ERROR("Could not open file: '{0}'", filepath);
        }

        return result;
    }

    std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
    {
        EVA_PROFILE_FUNCTION();

        std::unordered_map<GLenum, std::string> sources;

        const char* typeToken  = "//#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos             = source.find(typeToken, 0);
        while (pos != std::string::npos)
        {
            size_t eol = source.find_first_of("\r\n", pos);
            EVA_INTERNAL_ASSERT(eol != std::string::npos, "Syntax error");
            size_t begin     = pos + typeTokenLength + 1;
            std::string type = source.substr(begin, eol - begin);

            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            pos                = source.find(typeToken, nextLinePos);

            sources[ShaderTypeFromString(type)] =
              (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
        }

        return sources;
    }

    void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& sources)
    {
        EVA_PROFILE_FUNCTION();

        // Get a program object.
        GLuint program = glCreateProgram();

        std::array<GLenum, 5> shaderIds;
        int shaderIdIndex = 0;

        for (const auto& kv : sources)
        {
            GLenum type        = kv.first;
            const auto& source = kv.second;

            // Create an empty vertex shader handle
            GLuint shader = glCreateShader(type);

            // Send the vertex shader source code to GL
            // Note that std::string's .c_str is NULL character terminated.
            const GLchar* sourceCStr = source.c_str();
            glShaderSource(shader, 1, &sourceCStr, 0);

            // Compile the vertex shader
            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                // The maxLength includes the NULL character
                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                // We don't need the shader anymore.
                glDeleteShader(shader);

                EVA_INTERNAL_ERROR("{0}", infoLog.data());
                EVA_INTERNAL_ASSERT(false, "Shader compilation failure\n");
                break;
            }

            // Attach our shaders to our program
            glAttachShader(program, shader);
            shaderIds[shaderIdIndex++] = shader;
        }

        // Link our program
        glLinkProgram(program);

        // Note the different functions here: glGetProgram* instead of glGetShader*.
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            // We don't need the program anymore.
            glDeleteProgram(program);
            // Don't leak shaders either.
            for (auto id : shaderIds)
                glDeleteShader(id);

            EVA_INTERNAL_ASSERT(false, "Shader link failure\n {0}", infoLog.data());
            return;
        }

        // Always detach shaders after a successful link.
        for (auto id : shaderIds)
            glDetachShader(program, id);

        m_RendererId = program;
    }

    OpenGLShader::~OpenGLShader() { glDeleteProgram(m_RendererId); }

    void OpenGLShader::Bind() const { glUseProgram(m_RendererId); }

    void OpenGLShader::Unbind() const { glUseProgram(0); }

    void OpenGLShader::SetUniformBool(const std::string& name, const bool value)
    {
        auto location = GetUniformLocation(name);
        glUniform1i(location, value);
    }

    void OpenGLShader::SetUniformInt(const std::string& name, const int value)
    {
        auto location = GetUniformLocation(name);
        glUniform1i(location, value);
    }

    void OpenGLShader::SetUniformFloat(const std::string& name, const float value)
    {
        auto location = GetUniformLocation(name);
        glUniform1f(location, value);
    }

    void OpenGLShader::SetUniformFloat2(const std::string& name, const glm::vec2& value)
    {
        auto location = GetUniformLocation(name);
        glUniform2f(location, value.x, value.y);
    }

    void OpenGLShader::SetUniformFloat3(const std::string& name, const glm::vec3& value)
    {
        auto location = GetUniformLocation(name);
        glUniform3f(location, value.x, value.y, value.z);
    }

    void OpenGLShader::SetUniformFloat4(const std::string& name, const glm::vec4& value)
    {
        auto location = GetUniformLocation(name);
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }

    void OpenGLShader::SetUniformMat3(const std::string& name, const glm::mat3& matrix)
    {
        auto location = GetUniformLocation(name);
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::SetUniformMat4(const std::string& name, const glm::mat4& matrix)
    {
        auto location = GetUniformLocation(name);
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::BindTexture(const std::string& name, const Ref<Texture>& texture)
    {
        auto unit = m_TextureUnit++;
        SetUniformInt(name, unit);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(OpenGLTexture::GetGLTarget(texture->GetTarget()), texture->GetRendererId());
    }

    void OpenGLShader::BindTexture(const std::string& name, TextureTarget target, uint32_t rendererId)
    {
        auto unit = m_TextureUnit++;
        SetUniformInt(name, unit);
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(OpenGLTexture::GetGLTarget(target), rendererId);
    }

    void OpenGLShader::BindImageTexture(const std::string& name, const Ref<Texture>& texture)
    {
        auto location = GetUniformLocation(name);
        location = 0;
        glBindImageTexture(location, texture->GetRendererId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, OpenGLTexture::GetGLFormat(texture->GetFormat()));
    }

    void OpenGLShader::DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
        EVA_PROFILE_FUNCTION();
        glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
#ifdef EVA_DEBUG
        glFinish();
#endif // EVA_DEBUG
    }

    void OpenGLShader::DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ)
    {
        EVA_PROFILE_FUNCTION();
        glDispatchComputeGroupSizeARB(numGroupsX, numGroupsY, numGroupsZ, groupSizeX, groupSizeY, groupSizeZ);
#ifdef EVA_DEBUG
        glFinish();
#endif // EVA_DEBUG
    }

    GLint OpenGLShader::GetUniformLocation(const std::string& name)
    {
        auto it = m_UniformLocationMap.find(name);
        if (it != m_UniformLocationMap.end()) { return (*it).second; }

        const GLint location = glGetUniformLocation(m_RendererId, name.c_str());
#ifdef EVA_DEBUG
        if (location == -1) { EVA_INTERNAL_WARN("Invalid uniform name: {}", name); }
#endif
        m_UniformLocationMap[name] = location;
        return location;
    }
} // namespace EVA
