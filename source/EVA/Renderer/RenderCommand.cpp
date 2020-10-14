#include "RenderCommand.hpp"

#include "Platform/OpenGL/OpenGLRendererAPI.hpp"

namespace EVA
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}
