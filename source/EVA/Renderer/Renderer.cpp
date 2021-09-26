#include "Renderer.hpp"
#include "Environment.hpp"

namespace EVA
{
    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData();

    void Renderer::Init() 
    { 
        EVA_PROFILE_FUNCTION();

        RenderCommand::Init(); 
        
        s_SceneData->cameraUniformBuffer = UniformBuffer::Create(0, nullptr, sizeof(CameraUniformBufferData));
        s_SceneData->lightUniformBuffer  = UniformBuffer::Create(1, nullptr, sizeof(LightUniformBufferData));
        s_SceneData->objectUniformBuffer = UniformBuffer::Create(2, nullptr, sizeof(ObjectUniformBufferData));
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        EVA_PROFILE_FUNCTION();
        RenderCommand::SetViewport(0, 0, width, height);
    }

    void Renderer::BeginScene(const Camera& camera, Ref<Environment> environment, const std::vector<Light>& lights)
    {
        EVA_PROFILE_FUNCTION();

        s_SceneData->environment = environment;
        s_SceneData->lights      = lights;

        RenderCommand::SetCullMode(CullMode::Back);

        // Camera
        s_SceneData->cameraData.viewMatrix                  = camera.GetViewMatrix();
        s_SceneData->cameraData.projectionMatrix            = camera.GetProjectionMatrix();
        s_SceneData->cameraData.viewProjectionMatrix        = camera.GetViewProjectionMatrix();
        s_SceneData->cameraData.inverseViewProjectionMatrix = glm::inverse(camera.GetViewProjectionMatrix());

        auto invVm                             = glm::inverse(camera.GetViewMatrix());
        s_SceneData->cameraData.cameraPosition = glm::vec3(invVm[3][0], invVm[3][1], invVm[3][2]);

        s_SceneData->cameraData.enviromentRotation = s_SceneData->environment->rotation;

        s_SceneData->cameraUniformBuffer->Bind(0);
        s_SceneData->cameraUniformBuffer->SetData(&s_SceneData->cameraData, sizeof(CameraUniformBufferData));

        // Lights
        s_SceneData->lightData.numLights = glm::min(maxLights, lights.size());
        for (size_t i = 0; i < s_SceneData->lightData.numLights; i++) 
        {
            s_SceneData->lightData.lights[i] = lights[i].GetData();
        }
        s_SceneData->lightUniformBuffer->Bind(1);
        s_SceneData->lightUniformBuffer->SetData(&s_SceneData->lightData, sizeof(LightUniformBufferData));
    }

    void Renderer::EndScene() {}

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& model, const Ref<Material>& material)
    {
        EVA_PROFILE_FUNCTION();

        shader->Bind();

        shader->BindTexture("u_EnvironmentMap", s_SceneData->environment->GetEnvironmentMap());
        shader->BindTexture("u_IrradianceMap", s_SceneData->environment->GetIrradianceMap());
        shader->BindTexture("u_PrefilterMap", s_SceneData->environment->GetPreFilterMap());
        shader->BindTexture("u_BrdfLUT", s_SceneData->environment->GetBrdfLUT());

        s_SceneData->objectData.model = model;

        if (material)
        {
            material->Bind(shader);
            s_SceneData->objectData.tiling        = material->tiling;
            s_SceneData->objectData.heightScale   = material->heightScale;
            s_SceneData->objectData.enableParalax = material->enableParalax && material->height != nullptr;
            s_SceneData->objectData.paralaxClip   = material->paralaxClip;
        }

        s_SceneData->objectUniformBuffer->Bind(2);
        s_SceneData->objectUniformBuffer->SetData(&s_SceneData->objectData, sizeof(ObjectUniformBufferData));

        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }
} // namespace EVA
