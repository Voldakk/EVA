#include "ShaderStorageBuffer.hpp"

#include "Platform/OpenGL/OpenGLShaderStorageBuffer.hpp"
#include "Renderer.hpp"

namespace EVA
{
    Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(const void* data, uint32_t size, Usage usage)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLShaderStorageBuffer>(data, size, usage);
        }
        EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }
} // namespace EVA
