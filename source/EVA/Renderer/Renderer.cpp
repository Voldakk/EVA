#include "Renderer.hpp"
#include <Platform\OpenGL\OpenGLShader.hpp>

namespace EVA
{
    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData();

    void Renderer::Init() { RenderCommand::Init(); }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height) { RenderCommand::SetViewport(0, 0, width, height); }

    void Renderer::BeginScene(OrthographicCamera& camera) { s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix(); }

    void Renderer::EndScene() {}

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& model)
    {
        shader->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(shader)->SetUniformMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->SetUniformMat4("u_Model", model);

        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }
} // namespace EVA
