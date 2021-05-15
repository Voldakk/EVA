#include "Renderer.hpp"
#include "Environment.hpp"

namespace EVA
{
    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData();

    void Renderer::Init() { RenderCommand::Init(); }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height) { RenderCommand::SetViewport(0, 0, width, height); }

    void Renderer::BeginScene(const Camera& camera, Ref<Environment> environment, const std::vector<Light>& lights)
    {
        EVA_PROFILE_FUNCTION();

        RenderCommand::SetCullMode(CullMode::Back);
        s_SceneData->viewMatrix           = camera.GetViewMatrix();
        s_SceneData->projectionMatrix     = camera.GetProjectionMatrix();
        s_SceneData->viewProjectionMatrix = camera.GetViewProjectionMatrix();

        auto invVm                  = glm::inverse(camera.GetViewMatrix());
        s_SceneData->cameraPosition = glm::vec3(invVm[3][0], invVm[3][1], invVm[3][2]);

        s_SceneData->environment = environment;
        s_SceneData->lights      = lights;
    }

    void Renderer::EndScene() {}

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& model)
    {
        EVA_PROFILE_FUNCTION();

        shader->Bind();

        shader->SetUniformMat4("u_View", s_SceneData->viewMatrix);
        shader->SetUniformMat4("u_Projection", s_SceneData->projectionMatrix);
        shader->SetUniformMat4("u_ViewProjection", s_SceneData->viewProjectionMatrix);
        shader->SetUniformFloat3("u_CameraPosition", s_SceneData->cameraPosition);

        shader->BindTexture("u_EnvironmentMap", s_SceneData->environment->GetEnvironmentMap());
        shader->BindTexture("u_IrradianceMap", s_SceneData->environment->GetIrradianceMap());
        shader->BindTexture("u_PrefilterMap", s_SceneData->environment->GetPreFilterMap());
        shader->BindTexture("u_BrdfLUT", s_SceneData->environment->GetBrdfLUT());

        shader->SetUniformInt("u_NumLights", s_SceneData->lights.size());
        for (size_t i = 0; i < s_SceneData->lights.size(); i++)
        {
            const auto uniformName = "u_AllLights[" + std::to_string(i) + "].";
            s_SceneData->lights[i].SetUniforms(shader, uniformName);
        }

        shader->SetUniformMat4("u_Model", model);

        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }
} // namespace EVA
