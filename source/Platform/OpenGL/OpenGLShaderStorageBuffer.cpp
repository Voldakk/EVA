#include "OpenGLShaderStorageBuffer.hpp"

#include "OpenGL.hpp"
#include "OpenGLContext.hpp"

namespace EVA
{
    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(const void* data, uint32_t size, Usage usage) :
      m_Size(size), m_Usage(OpenGL::GetGLUsage(usage))
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glGenBuffers(1, &m_RendererId));
        EVA_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererId));
        EVA_GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, m_Usage));
    }

    OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDeleteBuffers(1, &m_RendererId));
    }

    void OpenGLShaderStorageBuffer::BufferData(const void* data, uint32_t size, uint32_t offset)
    {
        EVA_PROFILE_FUNCTION();

        EVA_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererId));

        if (offset + size > m_Size)
        {
            if (offset != 0) { EVA_INTERNAL_WARN("Offset is not applied when recreating buffer"); }
            m_Size = size;
            EVA_GL_CALL(glNamedBufferData(m_RendererId, size, data, m_Usage));
        }
        else
        {
            EVA_GL_CALL(glNamedBufferSubData(m_RendererId, offset, size, data));
        }
    }

    void* OpenGLShaderStorageBuffer::Map(Access access) const
    {
        EVA_PROFILE_FUNCTION();
        return EVA_GL_CALL(glMapNamedBuffer(m_RendererId, OpenGL::GetGLAccess(access)));
    }

    void OpenGLShaderStorageBuffer::Unmap() const
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glUnmapNamedBuffer(m_RendererId));
    }
} // namespace EVA
