#include "VertexArray.hpp"

#include "Renderer.hpp"
#include "Platform/OpenGL/OpenGLVertexArray.hpp"

namespace EVA
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexArray>();
		}
		EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
