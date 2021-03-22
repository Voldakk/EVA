#include "Framebuffer.hpp"
#include "Renderer.hpp"
#include "Platform/OpenGL/OpenGLFramebuffer.hpp"

namespace EVA
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: return nullptr;
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLFramebuffer>(spec);
		}

		EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
