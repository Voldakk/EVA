#include "OpenGLVertexArray.hpp"

#include <glad/glad.h>

namespace EVA
{
    [[nodiscard]] static GLenum ShaderDataTypeToGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Float: return GL_FLOAT;
            case ShaderDataType::Float2: return GL_FLOAT;
            case ShaderDataType::Float3: return GL_FLOAT;
            case ShaderDataType::Float4: return GL_FLOAT;
            case ShaderDataType::Mat3: return GL_FLOAT;
            case ShaderDataType::Mat4: return GL_FLOAT;
            case ShaderDataType::Int: return GL_INT;
            case ShaderDataType::Int2: return GL_INT;
            case ShaderDataType::Int3: return GL_INT;
            case ShaderDataType::Int4: return GL_INT;
            case ShaderDataType::Bool: return GL_BOOL;
        }

        EVA_INTERNAL_ASSERT(false, "Unknown ShaderDataType");
        return 0;
    }

    OpenGLVertexArray::OpenGLVertexArray() { glGenVertexArrays(1, &m_RendererId); }

    OpenGLVertexArray::~OpenGLVertexArray() { glDeleteVertexArrays(1, &m_RendererId); }

    void OpenGLVertexArray::Bind() const { glBindVertexArray(m_RendererId); }

    void OpenGLVertexArray::Unbind() const { glBindVertexArray(0); }

    void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& buffer)
    {
        EVA_INTERNAL_ASSERT(!buffer->GetLayout().GetElements().empty(), "Vertex buffer has no layout");

        glBindVertexArray(m_RendererId);
        buffer->Bind();

        uint32_t index     = 0;
        const auto& layout = buffer->GetLayout();
        for (const auto& element : layout)
        {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(index, element.GetComponentCount(), ShaderDataTypeToGLBaseType(element.type),
                                  element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)((size_t)element.offset));
            index++;
        }

        m_VertexBuffers.push_back(buffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& buffer)
    {
        glBindVertexArray(m_RendererId);
        buffer->Bind();

        m_IndexBuffer = buffer;
    }
} // namespace EVA
