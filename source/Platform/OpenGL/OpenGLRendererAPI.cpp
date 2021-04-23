#include "OpenGLRendererAPI.hpp"

#include <glad/glad.h>

namespace EVA
{
    void OpenGLRendererAPI::Init()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) { glViewport(x, y, width, height); }

    void OpenGLRendererAPI::Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

    void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) { glClearColor(color.r, color.g, color.b, color.a); }

    void OpenGLRendererAPI::SetCullMode(CullMode mode)
    {
        if (mode == CullMode::None)
            glDisable(GL_CULL_FACE);
        else
            glEnable(GL_CULL_FACE);

        switch (mode)
        {
            case EVA::CullMode::Back: glCullFace(GL_BACK); break;
            case EVA::CullMode::Front: glCullFace(GL_FRONT); break;
            case EVA::CullMode::Both: glCullFace(GL_FRONT_AND_BACK); break;
        }
    }

    void OpenGLRendererAPI::EnableDepth(bool value)
    {
        if (value)
            glDepthMask(GL_TRUE);
        else
            glDepthMask(GL_FALSE);
    }

    void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
    {
        glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }
} // namespace EVA
