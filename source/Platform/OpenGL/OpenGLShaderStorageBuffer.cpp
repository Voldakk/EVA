#include "OpenGLShaderStorageBuffer.hpp"
#include <glad/glad.h>

namespace EVA
{
    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(void* data, uint32_t size) : m_Size(size)
    {
        glGenBuffers(1, &m_RendererId);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererId);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STREAM_DRAW);
    }

    OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer() { glDeleteBuffers(1, &m_RendererId); }

    void OpenGLShaderStorageBuffer::BufferData(void* data, uint32_t size)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererId);

        if (size > m_Size)
        {
            m_Size = size;
            glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_STREAM_DRAW);
        }
        else
        {
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
        }
    }
} // namespace EVA
