#pragma once

#include "RendererAPI.hpp"
#include "RenderCommand.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"
#include "OrthographicCamera.hpp"

namespace EVA
{
	class Renderer
	{
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static SceneData* s_SceneData;

	public:
		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};
}
