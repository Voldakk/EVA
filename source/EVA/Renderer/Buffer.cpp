#include "Buffer.hpp"

#include "EVA/Core.hpp"
#include "Renderer.hpp"

#include "Platform/OpenGL/OpenGLBuffer.hpp"

namespace EVA
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLVertexBuffer(vertices, size);
		}
		EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLIndexBuffer(indices, size);
		}
		EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
