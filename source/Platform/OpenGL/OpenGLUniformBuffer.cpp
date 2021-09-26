#include "OpenGLUniformBuffer.hpp"

#include "OpenGL.hpp"
#include "OpenGLContext.hpp"

namespace EVA
{
    OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t binding, const void* data, uint32_t size, Usage usage) : m_Usage(OpenGL::GetGLUsage(usage))
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glCreateBuffers(1, &m_RendererId));
        EVA_GL_CALL(glNamedBufferData(m_RendererId, size, data, m_Usage));
        EVA_GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererId));
    }

    OpenGLUniformBuffer::~OpenGLUniformBuffer()
    {
        EVA_PROFILE_FUNCTION();
        glDeleteBuffers(1, &m_RendererId);
    }

    void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glNamedBufferSubData(m_RendererId, offset, size, data));
    }

    void OpenGLUniformBuffer::Bind(uint32_t binding) 
    { 
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererId));
    }

} // namespace EVA