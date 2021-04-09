#include "OpenGLShaderStorageBuffer.hpp"
#include <glad/glad.h>

namespace EVA
{
    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(void* data, uint32_t size) : m_Size(size)
    {
        glGenBuffers(1, &m_RendererId);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererId);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }

    OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer() { glDeleteBuffers(1, &m_RendererId); }

    void OpenGLShaderStorageBuffer::BufferData(void* data, uint32_t size, uint32_t offset)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererId);

        if (offset + size > m_Size)
        {
            if (offset != 0) {
                EVA_INTERNAL_WARN("Offset is not applied when recreating buffer");
            }
            m_Size = size;
            glNamedBufferData(m_RendererId, size, data, GL_DYNAMIC_DRAW);
        }
        else
        {
            glNamedBufferSubData(m_RendererId, offset, size, data);
        }
    }
} // namespace EVA
