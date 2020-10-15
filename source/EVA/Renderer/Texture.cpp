#include "Texture.hpp"

#include "Renderer.hpp"
#include "Platform/OpenGL/OpenGLTexture.hpp"

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
}