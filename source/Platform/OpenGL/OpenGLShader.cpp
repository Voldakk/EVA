#include "OpenGLShader.hpp"

#include "OpenGL.hpp"
#include "OpenGLTexture.hpp"
#include "OpenGLContext.hpp"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

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

    static const char* ShaderTypeToString(GLenum type)
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: return "vertex";
            case GL_FRAGMENT_SHADER: return "fragment";
            case GL_GEOMETRY_SHADER: return "geometry";
            case GL_TESS_CONTROL_SHADER: return "tess_control";
            case GL_TESS_EVALUATION_SHADER: return "tess_evaluation";
            case GL_COMPUTE_SHADER: return "compute";
        }
        EVA_INTERNAL_ASSERT(false, "Invalid shader type: '{}'", type);
        return nullptr;
    }

    static shaderc_shader_kind ShaderTypeToShaderC(GLenum type)
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: return shaderc_glsl_vertex_shader;
            case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
            case GL_GEOMETRY_SHADER: return shaderc_glsl_geometry_shader;
            case GL_TESS_CONTROL_SHADER: return shaderc_glsl_tess_control_shader;
            case GL_TESS_EVALUATION_SHADER: return shaderc_glsl_tess_evaluation_shader;
            case GL_COMPUTE_SHADER: return shaderc_glsl_compute_shader;
        }
        EVA_INTERNAL_ASSERT(false, "Invalid shader type: '{}'", type);
        return (shaderc_shader_kind)0;
    }

    static const char* ShaderTypeToCachedOpenGLFileExtension(GLenum type)
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: return ".gl.vert";
            case GL_FRAGMENT_SHADER: return ".gl.frag";
            case GL_GEOMETRY_SHADER: return ".gl.geom";
            case GL_TESS_CONTROL_SHADER: return ".gl.tesc";
            case GL_TESS_EVALUATION_SHADER: return ".gl.tese";
            case GL_COMPUTE_SHADER: return ".gl.comp";
        }
        EVA_INTERNAL_ASSERT(false, "Invalid shader type: '{}'", type);
        return "";
    }

    static const char* ShaderTypeToCachedVulkanFileExtension(GLenum type)
    {
        switch (type)
        {
            case GL_VERTEX_SHADER: return ".vk.vert";
            case GL_FRAGMENT_SHADER: return ".vk.frag";
            case GL_GEOMETRY_SHADER: return ".vk.geom";
            case GL_TESS_CONTROL_SHADER: return ".vk.tesc";
            case GL_TESS_EVALUATION_SHADER: return ".vk.tese";
            case GL_COMPUTE_SHADER: return ".vk.comp";
        }
        EVA_INTERNAL_ASSERT(false, "Invalid shader type: '{}'", type);
        return "";
    }

    static const char* GetCacheDirectory() 
    { 
        return "cache/shader/opengl"; 
    }

    static void CreateCacheDirectoryIfNeeded()
    {
        auto cacheDirectory = GetCacheDirectory();
        if (!FileSystem::Exists(cacheDirectory)) { FileSystem::CreateDirectories(cacheDirectory); }
    }

    OpenGLShader::OpenGLShader(const Path& filepath)
    {
        EVA_PROFILE_FUNCTION();

        m_Name       = filepath.filename().string();
        m_Path       = filepath;

        std::string source;
        bool res     = FileSystem::ReadFile(filepath, source);
        EVA_INTERNAL_ASSERT(res, "Failed to open shader source file {0}", FileSystem::ToString(filepath));

        auto sources = PreProcess(source);

        auto vulkanBinaries = CompileOrGetVulkanBinaries(sources);
        auto openGLBinaries = CompileOrGetOpenGLBinaries(vulkanBinaries);
        CreateProgram(openGLBinaries);
    }

    OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource) : m_Name(name)
    {
        EVA_PROFILE_FUNCTION();

        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER]   = vertexSource;
        sources[GL_FRAGMENT_SHADER] = fragmentSource;
        
        auto vulkanBinaries = CompileOrGetVulkanBinaries(sources);
        auto openGLBinaries = CompileOrGetOpenGLBinaries(vulkanBinaries);
        CreateProgram(openGLBinaries);
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

            sources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
        }

        return sources;
    }

    OpenGLShader::ShaderBinaries OpenGLShader::CompileOrGetVulkanBinaries(const ShaderSources& sources)
    {
        EVA_PROFILE_FUNCTION();

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        //options.SetOptimizationLevel(shaderc_optimization_level_performance);
        //options.SetAutoMapLocations(true);
        //options.SetAutoBindUniforms(true);

        Path cacheDirectory = GetCacheDirectory();
        CreateCacheDirectoryIfNeeded();

        ShaderBinaries vulkanBinaries;
        for (auto&& [type, source] : sources)
        {
            Path cachedPath = cacheDirectory / (FileSystem::ToSafeString(m_Path) + ShaderTypeToCachedVulkanFileExtension(type));
            auto& data      = vulkanBinaries[type];

            if (!FileSystem::ReadFile(cachedPath, data))
            {
                shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, ShaderTypeToShaderC(type), 
                    FileSystem::ToString(m_Path / ShaderTypeToString(type)).c_str(), options);

                if (module.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    auto message = module.GetErrorMessage();
                    EVA_INTERNAL_ASSERT(false, message);
                }

                data = std::vector<uint32_t>(module.cbegin(), module.cend());

                FileSystem::WriteFile(cachedPath, data);
            }
        }

        for (auto&& [type, data] : vulkanBinaries)
        {
            //Reflect(type, data);
        }

        return vulkanBinaries;
    }

    OpenGLShader::ShaderBinaries OpenGLShader::CompileOrGetOpenGLBinaries(const ShaderBinaries& vulkanBinaries)
    {
        EVA_PROFILE_FUNCTION();

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
        //options.SetOptimizationLevel(shaderc_optimization_level_performance);
        //options.SetAutoMapLocations(true);
        //options.SetAutoBindUniforms(true);

        Path cacheDirectory = GetCacheDirectory();
        CreateCacheDirectoryIfNeeded();

        ShaderBinaries glBinaries;
        ShaderSources glSources;
        for (auto&& [type, spirv] : vulkanBinaries)
        {
            Path cachedPath = cacheDirectory / (FileSystem::ToSafeString(m_Path) + ShaderTypeToCachedOpenGLFileExtension(type));
            auto& data      = glBinaries[type];

            if (!FileSystem::ReadFile(cachedPath, data))
            {
                spirv_cross::CompilerGLSL glslCompiler(spirv);
                glSources[type] = glslCompiler.compile();

                FileSystem::WriteFile(cacheDirectory / (FileSystem::ToSafeString(m_Path) + ".src" + ShaderTypeToCachedOpenGLFileExtension(type)),
                                      glSources[type]);

                shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(glSources[type], ShaderTypeToShaderC(type), 
                    FileSystem::ToString(m_Path / ShaderTypeToString(type)).c_str(), options);

                if (module.GetCompilationStatus() != shaderc_compilation_status_success)
                {
                    auto message = module.GetErrorMessage();
                    EVA_INTERNAL_ASSERT(false, message);
                }

                data = std::vector<uint32_t>(module.cbegin(), module.cend());

                FileSystem::WriteFile(cachedPath, data);
            }
        }

        return glBinaries;
    }

    void OpenGLShader::CreateProgram(const ShaderBinaries& glBinaries)
    {
        EVA_PROFILE_FUNCTION();

        GLuint program = glCreateProgram();

        std::vector<GLuint> shaderIDs;
        for (auto&& [type, spirv] : glBinaries)
        {
            EVA_GL_CALL(GLuint shaderId = glCreateShader(type));
            shaderIDs.emplace_back(shaderId);
            EVA_GL_CALL(glShaderBinary(1, &shaderId, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t)));
            EVA_GL_CALL(glSpecializeShader(shaderId, "main", 0, nullptr, nullptr));


            int isCompiled = 0;
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                GLint maxLength;
                EVA_GL_CALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength));

                std::vector<GLchar> infoLog(maxLength);
                EVA_GL_CALL(glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data()));
                EVA_INTERNAL_ERROR("Shader compilation failed ({0}):\n{1}", m_Path, infoLog.data());
            }
            else
            {
                EVA_GL_CALL(glAttachShader(program, shaderId));
            }
        }

        EVA_GL_CALL(glLinkProgram(program));

        GLint isLinked;
        EVA_GL_CALL(glGetProgramiv(program, GL_LINK_STATUS, &isLinked));
        if (isLinked == GL_FALSE)
        {
            GLint maxLength;
            EVA_GL_CALL(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength));

            std::vector<GLchar> infoLog(maxLength);
            EVA_GL_CALL(glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data()));
            EVA_INTERNAL_ERROR("Shader linking failed ({0}):\n{1}", m_Path, infoLog.data());

            EVA_GL_CALL(glDeleteProgram(program));

            for (auto id : shaderIDs)
            {
                EVA_GL_CALL(glDeleteShader(id));
            }
        }

        for (auto id : shaderIDs)
        {
            EVA_GL_CALL(glDetachShader(program, id));
            EVA_GL_CALL(glDeleteShader(id));
        }

        m_RendererId = program;
    }

    void OpenGLShader::Reflect(unsigned int type, const std::vector<uint32_t>& shaderData)
    {
        EVA_PROFILE_FUNCTION();

        spirv_cross::Compiler compiler(shaderData);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        EVA_INTERNAL_TRACE("OpenGLShader::Reflect - {0} {1}", ShaderTypeToString(type), m_Path);
        EVA_INTERNAL_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
        EVA_INTERNAL_TRACE("    {0} push constant buffers", resources.push_constant_buffers.size());
        EVA_INTERNAL_TRACE("    {0} sampled images", resources.sampled_images.size());

        EVA_INTERNAL_TRACE("Uniform buffers:");
        for (const auto& resource : resources.uniform_buffers)
        {
            const auto& bufferType = compiler.get_type(resource.base_type_id);
            uint32_t bufferSize    = compiler.get_declared_struct_size(bufferType);
            uint32_t binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
            int memberCount        = bufferType.member_types.size();

            EVA_INTERNAL_TRACE("  Name = {0}", resource.name);
            EVA_INTERNAL_TRACE("    Size = {0}", bufferSize);
            EVA_INTERNAL_TRACE("    Binding = {0}", binding);
            EVA_INTERNAL_TRACE("    Members = {0}", memberCount);
        }

        EVA_INTERNAL_TRACE("Push constant buffers:");
        for (const auto& resource : resources.push_constant_buffers)
        {
            const auto& bufferType = compiler.get_type(resource.base_type_id);
            uint32_t bufferSize    = compiler.get_declared_struct_size(bufferType);
            uint32_t binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
            int memberCount        = bufferType.member_types.size();

            EVA_INTERNAL_TRACE("  Name = {0}", resource.name);
            EVA_INTERNAL_TRACE("    Size = {0}", bufferSize);
            EVA_INTERNAL_TRACE("    Binding = {0}", binding);
            EVA_INTERNAL_TRACE("    Members = {0}", memberCount);
        }
    }

    OpenGLShader::~OpenGLShader()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDeleteProgram(m_RendererId));
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

    void OpenGLShader::BindImageTexture(const uint32_t location, const Ref<Texture>& texture, const Access access)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindImageTexture(location, texture->GetRendererId(), 0, GL_FALSE, 0, OpenGL::GetGLAccess(access),
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
