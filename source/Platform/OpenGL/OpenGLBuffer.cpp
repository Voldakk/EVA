#include "OpenGLBuffer.hpp"

#include "OpenGL.hpp"

namespace EVA
{
#pragma region OpenGLVertexBuffer

    OpenGLVertexBuffer::OpenGLVertexBuffer(const void* vertices, uint32_t size) : m_Size(size)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glCreateBuffers(1, &m_RendererId));
        EVA_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
        EVA_GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW));
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDeleteBuffers(1, &m_RendererId));
    }

    void OpenGLVertexBuffer::Bind() const
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_RendererId));
    }

    void OpenGLVertexBuffer::Unbind() const
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

#pragma endregion
#pragma region OpenGLIndexBuffer

    OpenGLIndexBuffer::OpenGLIndexBuffer(const std::vector<uint32_t>& indices) : m_Count(indices.size())
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glCreateBuffers(1, &m_RendererId));
        EVA_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId));
        EVA_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * m_Count, indices.data(), GL_STATIC_DRAW));
    }

    OpenGLIndexBuffer::~OpenGLIndexBuffer()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDeleteBuffers(1, &m_RendererId));
    }

    void OpenGLIndexBuffer::Bind() const
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererId));
    }

    void OpenGLIndexBuffer::Unbind() const
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

#pragma endregion
} // namespace EVA
