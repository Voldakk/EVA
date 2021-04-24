#include "Renderer.hpp"

namespace EVA
{
    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData();

    void Renderer::Init() { RenderCommand::Init(); }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height) { RenderCommand::SetViewport(0, 0, width, height); }

    void Renderer::BeginScene(const Camera& camera, Ref<Texture> environmentMap, Ref<Texture> irradianceMap, Ref<Texture> prefilterMap,
                              Ref<Texture> brdfLUT, const std::vector<Light>& lights)
    {
        RenderCommand::SetCullMode(CullMode::Back);
        s_SceneData->viewMatrix           = camera.GetViewMatrix();
        s_SceneData->projectionMatrix     = camera.GetProjectionMatrix();
        s_SceneData->viewProjectionMatrix = camera.GetViewProjectionMatrix();

        auto& vm                    = camera.GetViewMatrix();
        s_SceneData->cameraPosition = glm::vec3(vm[0][3], vm[1][3], vm[2][3]);

        s_SceneData->environmentMap = environmentMap;
        s_SceneData->irradianceMap  = irradianceMap;
        s_SceneData->irradianceMap  = irradianceMap;
        s_SceneData->prefilterMap   = prefilterMap;
        s_SceneData->brdfLUT        = brdfLUT;

        s_SceneData->lights = lights;
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

        shader->BindTexture("u_EnvironmentMap", s_SceneData->environmentMap);
        shader->BindTexture("u_IrradianceMap", s_SceneData->irradianceMap);
        shader->BindTexture("u_PrefilterMap", s_SceneData->prefilterMap);
        shader->BindTexture("u_BrdfLUT", s_SceneData->brdfLUT);

        shader->SetUniformInt("u_NumLights", s_SceneData->lights.size());
        for (size_t i = 0; i < s_SceneData->lights.size(); i++)
        {
            const auto uniformName = "u_AllLights[" + std::to_string(i) + "].";
            s_SceneData->lights[i].SetUniforms(shader, uniformName, i);
        }

        shader->SetUniformMat4("u_Model", model);

        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }
} // namespace EVA
