#include "OpenGLRendererAPI.hpp"

#include "OpenGL.hpp"

namespace EVA
{
    void OpenGLRendererAPI::Init()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glEnable(GL_DEPTH_TEST));
        EVA_GL_CALL(glEnable(GL_CULL_FACE));
        EVA_GL_CALL(glEnable(GL_BLEND));
        EVA_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        EVA_GL_CALL(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
    }

    void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glViewport(x, y, width, height));
    }

    void OpenGLRendererAPI::Clear()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    }

    void OpenGLRendererAPI::SetCullMode(CullMode mode)
    {
        EVA_PROFILE_FUNCTION();

        if (mode == CullMode::None) { EVA_GL_CALL(glDisable(GL_CULL_FACE)); }
        else
        {
            EVA_GL_CALL(glEnable(GL_CULL_FACE));
        }

        switch (mode)
        {
            case EVA::CullMode::Back: EVA_GL_CALL(glCullFace(GL_BACK)); break;
            case EVA::CullMode::Front: EVA_GL_CALL(glCullFace(GL_FRONT)); break;
            case EVA::CullMode::Both: EVA_GL_CALL(glCullFace(GL_FRONT_AND_BACK)); break;
            default: break;
        }
    }

    void OpenGLRendererAPI::EnableDepth(bool value)
    {
        EVA_PROFILE_FUNCTION();

        if (value) { EVA_GL_CALL(glDepthMask(GL_TRUE)); }
        else
        {
            EVA_GL_CALL(glDepthMask(GL_FALSE));
        }
    }

    void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr));
    }
} // namespace EVA
