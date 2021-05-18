#include "Buffer.hpp"

#include "Platform/OpenGL/OpenGLBuffer.hpp"
#include "Renderer.hpp"

namespace EVA
{
    Ref<VertexBuffer> VertexBuffer::Create(const void* vertices, uint32_t size)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(vertices, size);
        }
        EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(const std::vector<Vertex>& vertices)
    {
        return Create(vertices.data(), sizeof(Vertex) * vertices.size());
    }

    Ref<IndexBuffer> IndexBuffer::Create(const std::vector<uint32_t>& indices)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: return nullptr;
            case RendererAPI::API::OpenGL: return CreateRef<OpenGLIndexBuffer>(indices);
        }
        EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
        return nullptr;
    }
} // namespace EVA
