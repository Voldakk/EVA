#include "VertexArray.hpp"

#include "EVA/Core.hpp"
#include "Renderer.hpp"
#include "Platform/OpenGL/OpenGLVertexArray.hpp"

namespace EVA
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: return nullptr;
		case RendererAPI::API::OpenGL: return new OpenGLVertexArray();
		}
		EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}