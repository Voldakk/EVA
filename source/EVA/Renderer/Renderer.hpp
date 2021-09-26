#pragma once

#include "Camera.hpp"
#include "RenderCommand.hpp"
#include "RendererAPI.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"
#include "Light.hpp"
#include "UniformBuffer.hpp"
#include "Material.hpp"

namespace EVA
{
    struct CameraUniformBufferData
    {
        glm::mat4 viewProjectionMatrix;
        glm::mat4 inverseViewProjectionMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::vec3 cameraPosition;
        float enviromentRotation;
    };

    constexpr size_t maxLights = 10;
    struct LightUniformBufferData
    {
        int numLights, offset1, offset2, offset3;
        LightData lights[maxLights];
    };

    struct ObjectUniformBufferData
    {
        glm::mat4 model;
        glm::vec2 tiling;
        float heightScale;
        bool enableParalax;
        bool paralaxClip;
    };

    class Environment;
    class Renderer
    {
        struct SceneData
        {
            Ref<Environment> environment;
            std::vector<Light> lights;

            Ref<UniformBuffer> cameraUniformBuffer;
            CameraUniformBufferData cameraData;

            Ref<UniformBuffer> lightUniformBuffer;
            LightUniformBufferData lightData;

            Ref<UniformBuffer> objectUniformBuffer;
            ObjectUniformBufferData objectData;
        };

        static SceneData* s_SceneData;

      public:
        static void Init();

        static void OnWindowResize(uint32_t width, uint32_t height);

        static void BeginScene(const Camera& camera, Ref<Environment> environment, const std::vector<Light>& lights = {});
        static void EndScene();

        static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& model = glm::mat4(1.0f),
                           const Ref<Material>& material = nullptr);

        inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
    };
} // namespace EVA
