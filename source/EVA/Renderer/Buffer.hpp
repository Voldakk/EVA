#pragma once

#include "Vertex.hpp"

namespace EVA
{
    enum class ShaderDataType
    {
        None = 0,
        Float,
        Float2,
        Float3,
        Float4,
        Mat3,
        Mat4,
        Int,
        Int2,
        Int3,
        Int4,
        Bool
    };

    static uint32_t ShaderDataTypeSize(ShaderDataType type)
    {
        switch (type)
        {
            case EVA::ShaderDataType::Float: return 4 * 1;
            case EVA::ShaderDataType::Float2: return 4 * 2;
            case EVA::ShaderDataType::Float3: return 4 * 3;
            case EVA::ShaderDataType::Float4: return 4 * 4;
            case EVA::ShaderDataType::Mat3: return 4 * 3 * 3;
            case EVA::ShaderDataType::Mat4: return 4 * 4 * 4;
            case EVA::ShaderDataType::Int: return 4 * 1;
            case EVA::ShaderDataType::Int2: return 4 * 2;
            case EVA::ShaderDataType::Int3: return 4 * 3;
            case EVA::ShaderDataType::Int4: return 4 * 4;
            case EVA::ShaderDataType::Bool: return 1;
            case EVA::ShaderDataType::None: return 0;
        }

        EVA_INTERNAL_ASSERT(false, "Unknown ShaderDataType");
        return 0;
    }

    struct BufferElement
    {
        std::string name;
        ShaderDataType type;
        uint32_t size;
        uint32_t offset;
        bool normalized;

        BufferElement() : name(""), type(ShaderDataType::None), size(0), offset(0), normalized(false) {}

        BufferElement(ShaderDataType type, const std::string& name, bool normalized = false) :
          name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized)
        {
        }

        inline uint32_t GetComponentCount() const
        {
            switch (type)
            {
                case EVA::ShaderDataType::Float: return 1;
                case EVA::ShaderDataType::Float2: return 2;
                case EVA::ShaderDataType::Float3: return 3;
                case EVA::ShaderDataType::Float4: return 4;
                case EVA::ShaderDataType::Mat3: return 3 * 3;
                case EVA::ShaderDataType::Mat4: return 4 * 4;
                case EVA::ShaderDataType::Int: return 1;
                case EVA::ShaderDataType::Int2: return 2;
                case EVA::ShaderDataType::Int3: return 3;
                case EVA::ShaderDataType::Int4: return 4;
                case EVA::ShaderDataType::Bool: return 1;
                case EVA::ShaderDataType::None: return 0;
            }

            EVA_INTERNAL_ASSERT(false, "Unknown ShaderDataType");
            return 0;
        }
    };

    class BufferLayout
    {
        std::vector<BufferElement> m_Elements;
        uint32_t m_Stride;

      public:
        BufferLayout() : m_Stride(0) {}
        BufferLayout(const std::initializer_list<BufferElement>& elements) : m_Elements(elements) { CalculateOffsetsAndStride(); }

        inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
        inline const uint32_t GetStride() const { return m_Stride; }

        std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
        std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
        std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
        std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

      private:
        void CalculateOffsetsAndStride()
        {
            uint32_t offset = 0;
            m_Stride        = 0;
            for (auto& element : m_Elements)
            {
                element.offset = offset;
                offset += element.size;
                m_Stride += element.size;
            }
        }
    };

    class VertexBuffer
    {
      public:
        virtual ~VertexBuffer() {};

        virtual void Bind() const   = 0;
        virtual void Unbind() const = 0;


        virtual void SetLayout(const BufferLayout& layout) = 0;
        virtual const BufferLayout& GetLayout() const      = 0;

        static Ref<VertexBuffer> Create(const void* vertices, uint32_t size);
        static Ref<VertexBuffer> Create(const std::vector<Vertex>& vertices);
    };

    class IndexBuffer
    {
      public:
        virtual ~IndexBuffer() {};

        virtual void Bind() const   = 0;
        virtual void Unbind() const = 0;

        virtual uint32_t GetCount() const = 0;

        static Ref<IndexBuffer> Create(const std::vector<uint32_t>& indices);
    };
} // namespace EVA
