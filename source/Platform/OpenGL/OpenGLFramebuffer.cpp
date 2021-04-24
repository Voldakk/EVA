#include "OpenGLFramebuffer.hpp"
#include "OpenGLTexture.hpp"
#include <glad/glad.h>

namespace EVA
{
    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : m_Specification(spec)
    {
        EVA_PROFILE_FUNCTION();
        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        glDeleteFramebuffers(1, &m_RendererId);
        glDeleteTextures(1, &m_ColorAttachment);
        glDeleteTextures(1, &m_DepthAttachment);
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_RendererId != 0)
        {
            glDeleteFramebuffers(1, &m_RendererId);
            glDeleteTextures(1, &m_ColorAttachment);
            glDeleteTextures(1, &m_DepthAttachment);
        }

        glCreateFramebuffers(1, &m_RendererId);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment);
        glBindTexture(GL_TEXTURE_2D, m_ColorAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, OpenGLTexture::GetGLFormat(m_Specification.format), m_Specification.width, m_Specification.height, 0,
                     OpenGLTexture::GetGLFormat(GetTextureFormat(m_Specification.format)),
                     OpenGLTexture::GetGLDataType(GetTextureDataType(m_Specification.format)), nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);

        glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
        glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Specification.width, m_Specification.height);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

        EVA_INTERNAL_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
        glViewport(0, 0, m_Specification.width, m_Specification.height);
    }

    void OpenGLFramebuffer::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
    {
        EVA_PROFILE_FUNCTION();

        if (width == 0 || height == 0 || width > MaxFramebufferSize || height > MaxFramebufferSize)
        {
            EVA_INTERNAL_WARN("Invalid framebuffer size: {0}, {1}", width, height);
        }
        m_Specification.width  = width;
        m_Specification.height = height;
        Invalidate();
    }
    void OpenGLFramebuffer::ResetTexturesAttachments() 
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);
    }

    void OpenGLFramebuffer::AttachTexture(std::shared_ptr<Texture> texture, int mip)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (GLenum)texture->GetTarget(), texture->GetRendererId(), mip);
    }

    void OpenGLFramebuffer::AttachCubemap(std::shared_ptr<Texture> cubemap, int sideIndex, int mip)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + sideIndex, cubemap->GetRendererId(), mip);
    }

    
} // namespace EVA
