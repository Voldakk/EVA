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

		static void Init();

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();

		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& model = glm::mat4(1.0f));

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};
}