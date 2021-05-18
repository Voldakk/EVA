#include "OpenGLShaderStorageBuffer.hpp"
#include "OpenGL.hpp"

namespace EVA
{
    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(void* data, uint32_t size) : m_Size(size)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glGenBuffers(1, &m_RendererId));
        EVA_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererId));
        EVA_GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW));
    }

    OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDeleteBuffers(1, &m_RendererId));
    }

    void OpenGLShaderStorageBuffer::BufferData(void* data, uint32_t size, uint32_t offset)
    {
        EVA_PROFILE_FUNCTION();

        EVA_GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererId));

        if (offset + size > m_Size)
        {
            if (offset != 0) { EVA_INTERNAL_WARN("Offset is not applied when recreating buffer"); }
            m_Size = size;
            EVA_GL_CALL(glNamedBufferData(m_RendererId, size, data, GL_DYNAMIC_DRAW));
        }
        else
        {
            EVA_GL_CALL(glNamedBufferSubData(m_RendererId, offset, size, data));
        }
    }
} // namespace EVA
