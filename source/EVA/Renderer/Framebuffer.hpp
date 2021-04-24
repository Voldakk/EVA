#pragma once

#include "EVA/Core/Core.hpp"
#include "Texture.hpp"

namespace EVA
{
    constexpr uint32_t MaxFramebufferSize = 16384;

    struct FramebufferSpecification
    {
        uint32_t width, height;
        uint16_t samples     = 1;
        bool swapChainTarget = false;
        TextureFormat format = TextureFormat::RGBA16F;
    };

    class Framebuffer
    {
      public:
        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

        virtual ~Framebuffer() = default;

        virtual void Bind()   = 0;
        virtual void Unbind() = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;

        virtual void ResetTexturesAttachments()                                                  = 0;
        virtual void AttachTexture(std::shared_ptr<Texture> texture, int mip = 0)                = 0;
        virtual void AttachCubemap(std::shared_ptr<Texture> cubemap, int sideIndex, int mip = 0) = 0;

        virtual uint32_t GetColorAttachmentRendererId() const            = 0;
        virtual uint32_t GetDepthAttachmentRendererId() const            = 0;
        virtual const FramebufferSpecification& GetSpecification() const = 0;
    };
} // namespace EVA
