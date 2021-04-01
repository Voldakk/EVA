#include "Texture.hpp"

#include "Platform/OpenGL/OpenGLTexture.hpp"
#include "Renderer.hpp"

namespace EVA
{
    Ref<Texture2D> Texture2D::Create(const std::string& path)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(path);
        }

        EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const uint32_t width, const uint32_t height)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLTexture2D>(width, height);
        }

        EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }
} // namespace EVA