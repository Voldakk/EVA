#include "UniformBuffer.hpp"

#include "Renderer.hpp"
#include "Platform/OpenGL/OpenGLUniformBuffer.hpp"

namespace EVA
{
    Ref<UniformBuffer> UniformBuffer::Create(uint32_t binding, const void* data, uint32_t size, Usage usage)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLUniformBuffer>(binding, data, size, usage);
        }

        EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
} // namespace EVA
