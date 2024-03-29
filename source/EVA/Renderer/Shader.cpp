#include "Shader.hpp"

#include "Platform/OpenGL/OpenGLShader.hpp"
#include "Renderer.hpp"
#include "EVA/Assets.hpp"

namespace EVA
{
    Ref<Shader> Shader::Create(const Path& path)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLShader>(path);
        }
        EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLShader>(name, vertexSource, fragmentSource);
        }
        EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    //// ShaderLibrary ////

    void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
    {
        EVA_INTERNAL_ASSERT(!Exists(name), "Shader already exists!");
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Add(const Ref<Shader>& shader)
    {
        auto& name = shader->GetName();
        Add(name, shader);
    }

    Ref<Shader> ShaderLibrary::Load(const std::string& filepath)
    {
        auto shader = AssetManager::Load<Shader>(filepath);
        Add(shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
    {
        auto shader = AssetManager::Load<Shader>(filepath);
        Add(name, shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::Get(const std::string& name)
    {
        EVA_INTERNAL_ASSERT(Exists(name), "Shader not found!");
        return m_Shaders[name];
    }

    bool ShaderLibrary::Exists(const std::string& name) const { return m_Shaders.find(name) != m_Shaders.end(); }
} // namespace EVA
