#pragma once

#include "RendererAPI.hpp"
#include "RenderCommand.hpp"
#include "VertexArray.hpp"

namespace EVA
{
	class Renderer
	{
	public:
		static void BeginScene();
		static void EndScene();

		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};
}
