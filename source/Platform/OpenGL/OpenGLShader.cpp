#include "OpenGLShader.hpp"

#include "OpenGL.hpp"
#include "OpenGLTexture.hpp"
#include <fstream>

namespace EVA
{
    static GLenum ShaderTypeFromString(const std::string& type)
    {
        EVA_PROFILE_FUNCTION();

        if (type == "vertex") return GL_VERTEX_SHADER;
        if (type == "fragment") return GL_FRAGMENT_SHADER;
        if (type == "geometry") return GL_GEOMETRY_SHADER;
        if (type == "tess_control") return GL_TESS_CONTROL_SHADER;
        if (type == "tess_evaluation") return GL_TESS_EVALUATION_SHADER;
        if (type == "compute") return GL_COMPUTE_SHADER;

        EVA_INTERNAL_ASSERT(false, "Invalid shader type: '{}'", type);
        return 0;
    }

    OpenGLShader::OpenGLShader(const Path& filepath)
    {
        EVA_PROFILE_FUNCTION();

        m_Name       = filepath.filename().string();
        m_Path       = filepath;
        auto source  = ReadFile(filepath);
        auto sources = PreProcess(source);
        Compile(sources);
    }

    OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource) : m_Name(name)
    {
        EVA_PROFILE_FUNCTION();

        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER]   = vertexSource;
        sources[GL_FRAGMENT_SHADER] = fragmentSource;
        Compile(sources);
    }

    std::string OpenGLShader::ReadFile(const Path& filepath)
    {
        EVA_PROFILE_FUNCTION();

        std::ifstream in;
        bool res = FileSystem::OpenFile(in, filepath, std::ios::in | std::ios::binary);

        EVA_INTERNAL_ASSERT(res, "Could not open file: '{}'", filepath);

        std::string result;
        in.seekg(0, std::ios::end);
        result.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&result[0], result.size());
        in.close();

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
        EVA_GL_CALL(GLuint program = glCreateProgram());

        std::vector<GLenum> shaderIds(sources.size());
        int shaderIdIndex = 0;

        for (const auto& kv : sources)
        {
            GLenum type        = kv.first;
            const auto& source = kv.second;

            // Create an empty vertex shader handle
            EVA_GL_CALL(GLuint shader = glCreateShader(type));

            // Send the vertex shader source code to GL
            // Note that std::string's .c_str is NULL character terminated.
            const GLchar* sourceCStr = source.c_str();
            EVA_GL_CALL(glShaderSource(shader, 1, &sourceCStr, 0));

            // Compile the vertex shader
            EVA_GL_CALL(glCompileShader(shader));

            GLint isCompiled = 0;
            EVA_GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled));
            if (isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                EVA_GL_CALL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength));

                // The maxLength includes the NULL character
                std::vector<GLchar> infoLog(maxLength);
                EVA_GL_CALL(glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]));

                // We don't need the shader anymore.
                EVA_GL_CALL(glDeleteShader(shader));

                EVA_INTERNAL_ERROR("{0}", infoLog.data());
                EVA_INTERNAL_ASSERT(false, "Shader compilation failure\n");
                break;
            }

            // Attach our shaders to our program
            EVA_GL_CALL(glAttachShader(program, shader));
            shaderIds[shaderIdIndex++] = shader;
        }

        // Link our program
        EVA_GL_CALL(glLinkProgram(program));

        // Note the different functions here: glGetProgram* instead of glGetShader*.
        GLint isLinked = 0;
        EVA_GL_CALL(glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked));
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            EVA_GL_CALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength));

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            EVA_GL_CALL(glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]));

            // We don't need the program anymore.
            EVA_GL_CALL(glDeleteProgram(program));
            // Don't leak shaders either.
            for (auto id : shaderIds)
            {
                EVA_GL_CALL(glDeleteShader(id));
            }

            EVA_INTERNAL_ASSERT(false, "Shader link failure\n {0}", infoLog.data());
            return;
        }

        // Always detach shaders after a successful link.
        for (auto id : shaderIds)
        {
            EVA_GL_CALL(glDetachShader(program, id));
        }
        m_RendererId = program;
    }
    static std::unordered_set<uint32_t> s_Deleted;
    OpenGLShader::~OpenGLShader()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDeleteProgram(1234));
    }

    void OpenGLShader::Bind() const
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glUseProgram(m_RendererId));
    }

    void OpenGLShader::Unbind() const
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glUseProgram(0));
    }

    void OpenGLShader::SetUniformBool(const std::string& name, const bool value)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniform1i(location, value));
    }

    void OpenGLShader::SetUniformInt(const std::string& name, const int value)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniform1i(location, value));
    }

    void OpenGLShader::SetUniformInt2(const std::string& name, const glm::ivec2& value)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniform2i(location, value.x, value.y));
    }

    void OpenGLShader::SetUniformInt3(const std::string& name, const glm::ivec3& value)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniform3i(location, value.x, value.y, value.z));
    }

    void OpenGLShader::SetUniformInt4(const std::string& name, const glm::ivec4& value)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniform4i(location, value.x, value.y, value.z, value.w));
    }

    void OpenGLShader::SetUniformFloat(const std::string& name, const float value)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniform1f(location, value));
    }

    void OpenGLShader::SetUniformFloat2(const std::string& name, const glm::vec2& value)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniform2f(location, value.x, value.y));
    }

    void OpenGLShader::SetUniformFloat3(const std::string& name, const glm::vec3& value)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniform3f(location, value.x, value.y, value.z));
    }

    void OpenGLShader::SetUniformFloat4(const std::string& name, const glm::vec4& value)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniform4f(location, value.x, value.y, value.z, value.w));
    }

    void OpenGLShader::SetUniformMat3(const std::string& name, const glm::mat3& matrix)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix)));
    }

    void OpenGLShader::SetUniformMat4(const std::string& name, const glm::mat4& matrix)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        EVA_GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)));
    }

    void OpenGLShader::BindTexture(const std::string& name, const Ref<Texture>& texture)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        if (location == -1) return;

        auto unit = m_TextureUnit++;
        EVA_GL_CALL(glUniform1i(location, unit));
        EVA_GL_CALL(glActiveTexture(GL_TEXTURE0 + unit));
        EVA_GL_CALL(glBindTexture(OpenGLTexture::GetGLTarget(texture->GetTarget()), texture->GetRendererId()));
    }

    void OpenGLShader::BindTexture(const std::string& name, const TextureTarget target, const uint32_t rendererId)
    {
        EVA_PROFILE_FUNCTION();
        auto location = GetUniformLocation(name);
        if (location == -1) return;

        auto unit = m_TextureUnit++;
        EVA_GL_CALL(glUniform1i(location, unit));
        EVA_GL_CALL(glActiveTexture(GL_TEXTURE0 + unit));
        EVA_GL_CALL(glBindTexture(OpenGLTexture::GetGLTarget(target), rendererId));
    }

    void OpenGLShader::BindImageTexture(const uint32_t location, const Ref<Texture>& texture, const TextureAccess access)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindImageTexture(location, texture->GetRendererId(), 0, GL_FALSE, 0, OpenGLTexture::GetGLAccess(access),
                                       OpenGLTexture::GetGLFormat(texture->GetFormat())));
    }

    void OpenGLShader::BindStorageBuffer(const uint32_t index, const Ref<ShaderStorageBuffer>& buffer)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer->GetRendererId()));
    }

    void OpenGLShader::DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ));
#ifdef EVA_DEBUG
        EVA_GL_CALL(glFinish());
#endif // EVA_DEBUG
    }

    void OpenGLShader::DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ, uint32_t groupSizeX,
                                       uint32_t groupSizeY, uint32_t groupSizeZ)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDispatchComputeGroupSizeARB(numGroupsX, numGroupsY, numGroupsZ, groupSizeX, groupSizeY, groupSizeZ));
#ifdef EVA_DEBUG
        EVA_GL_CALL(glFinish());
#endif // EVA_DEBUG
    }

    GLint OpenGLShader::GetUniformLocation(const std::string& name)
    {
        auto it = m_UniformLocationMap.find(name);
        if (it != m_UniformLocationMap.end()) { return (*it).second; }

        EVA_GL_CALL(const GLint location = glGetUniformLocation(m_RendererId, name.c_str()));
#ifdef EVA_DEBUG
        if (location == -1) { EVA_INTERNAL_WARN("{} - Invalid uniform name: {}", m_Name, name); }
#endif
        m_UniformLocationMap[name] = location;
        return location;
    }
} // namespace EVA
