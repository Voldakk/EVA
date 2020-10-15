#pragma once

#include "EVA/Renderer/RendererAPI.hpp"

namespace EVA
{
	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		OpenGLRendererAPI() = default;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear()  override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
	};
}
