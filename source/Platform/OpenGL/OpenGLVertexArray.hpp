#pragma once

#include "EVA/Renderer/VertexArray.hpp"

namespace EVA
{
	class OpenGLVertexArray : public VertexArray
	{
		uint32_t m_RendererId;

		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer) override;

		inline virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
		inline virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override { return m_IndexBuffer; }
	};
}
