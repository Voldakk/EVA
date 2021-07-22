#include "OpenGLVertexArray.hpp"

#include "OpenGL.hpp"

namespace EVA
{
    static GLenum ShaderDataTypeToGLBaseType(ShaderDataType type)
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
            case ShaderDataType::None: return 0;
        }
        throw;
    }

    OpenGLVertexArray::OpenGLVertexArray()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glGenVertexArrays(1, &m_RendererId));
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glDeleteVertexArrays(1, &m_RendererId));
    }

    void OpenGLVertexArray::Bind() const
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindVertexArray(m_RendererId));
    }

    void OpenGLVertexArray::Unbind() const
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindVertexArray(0));
    }

    void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& buffer)
    {
        EVA_PROFILE_FUNCTION();

        EVA_INTERNAL_ASSERT(!buffer->GetLayout().GetElements().empty(), "Vertex buffer has no layout");

        EVA_GL_CALL(glBindVertexArray(m_RendererId));
        buffer->Bind();

        uint32_t index     = 0;
        const auto& layout = buffer->GetLayout();
        for (const auto& element : layout)
        {
            EVA_GL_CALL(glEnableVertexAttribArray(index));
            EVA_GL_CALL(glVertexAttribPointer(index, element.GetComponentCount(), ShaderDataTypeToGLBaseType(element.type),
                                              element.normalized ? GL_TRUE : GL_FALSE, layout.GetStride(), (const void*)((size_t)element.offset)));
            index++;
        }

        m_VertexBuffers.push_back(buffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& buffer)
    {
        EVA_PROFILE_FUNCTION();

        EVA_GL_CALL(glBindVertexArray(m_RendererId));
        buffer->Bind();

        m_IndexBuffer = buffer;
    }
} // namespace EVA
