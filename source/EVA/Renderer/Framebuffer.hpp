#pragma once

#include "EVA/Core/Core.hpp"
#include <glm/glm.hpp>

namespace EVA
{
	struct FramebufferSpecification
	{
		uint32_t width, height;
		uint16_t Samples = 1; 
		bool swapChainTarget = false;
	};

	class Framebuffer {
	public:
		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererId() const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;
	};
}
