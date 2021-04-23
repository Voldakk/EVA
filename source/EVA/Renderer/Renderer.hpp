#pragma once

#include "Camera.hpp"
#include "RenderCommand.hpp"
#include "RendererAPI.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"

namespace EVA
{
    class Renderer
    {
        struct SceneData
        {
            glm::vec3 cameraPosition;

            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            glm::mat4 viewProjectionMatrix;

            Ref<Texture> environmentMap;
            Ref<Texture> irradianceMap;
            Ref<Texture> prefilterMap;
            Ref<Texture> brdfLUT;
        };

        static SceneData* s_SceneData;

      public:
        static void Init();

        static void OnWindowResize(uint32_t width, uint32_t height);

        static void BeginScene(const Camera& camera, Ref<Texture> environmentMap, Ref<Texture> irradianceMap, Ref<Texture> prefilterMap,
                               Ref<Texture> brdfLUT);
        static void EndScene();

        static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& model = glm::mat4(1.0f));

        inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
    };
} // namespace EVA
