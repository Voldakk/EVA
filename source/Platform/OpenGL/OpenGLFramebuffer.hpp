#pragma once

#include "EVA/Renderer/Framebuffer.hpp"

namespace EVA
{
    class OpenGLFramebuffer : public Framebuffer
    {
      public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        virtual ~OpenGLFramebuffer();

        void Invalidate();

        void Bind() override;
        void Unbind() override;

        void Resize(uint32_t width, uint32_t height) override;

        void ResetTexturesAttachments() override;
        void AttachTexture(std::shared_ptr<Texture> texture, int mip = 0) override;
        void AttachCubemap(std::shared_ptr<Texture> cubemap, int sideIndex, int mip = 0) override;

        uint32_t GetColorAttachmentRendererId() const override { return m_ColorAttachment; };
        const FramebufferSpecification& GetSpecification() const override { return m_Specification; };

      private:
        uint32_t m_RendererId      = 0;
        uint32_t m_ColorAttachment = 0;
        uint32_t m_DepthAttachment = 0;
        FramebufferSpecification m_Specification;
    };
} // namespace EVA
