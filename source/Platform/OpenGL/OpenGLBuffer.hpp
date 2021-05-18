#include "EVA/Renderer/Buffer.hpp"

namespace EVA
{
    class OpenGLVertexBuffer : public VertexBuffer
    {
        uint32_t m_RendererId;
        uint32_t m_Size;

        BufferLayout m_Layout;

      public:
        OpenGLVertexBuffer(const void* vertices, uint32_t size);
        virtual ~OpenGLVertexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        inline virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };
        inline virtual const BufferLayout& GetLayout() const override { return m_Layout; };
    };

    class OpenGLIndexBuffer : public IndexBuffer
    {
        uint32_t m_RendererId;
        uint32_t m_Count;

      public:
        OpenGLIndexBuffer(const std::vector<uint32_t>& indices);
        virtual ~OpenGLIndexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        inline virtual uint32_t GetCount() const override { return m_Count; }
    };
} // namespace EVA
