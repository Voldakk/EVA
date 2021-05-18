#include "OpenGLFramebuffer.hpp"
#include "OpenGLTexture.hpp"
#include "OpenGL.hpp"

namespace EVA
{
    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : m_Specification(spec)
    {
        EVA_PROFILE_FUNCTION();
        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        EVA_PROFILE_FUNCTION();

        EVA_GL_CALL(glDeleteFramebuffers(1, &m_RendererId));
        EVA_GL_CALL(glDeleteTextures(1, &m_ColorAttachment));
        EVA_GL_CALL(glDeleteTextures(1, &m_DepthAttachment));
    }

    void OpenGLFramebuffer::Invalidate()
    {
        EVA_PROFILE_FUNCTION();

        if (m_RendererId != 0)
        {
            EVA_GL_CALL(glDeleteFramebuffers(1, &m_RendererId));
            EVA_GL_CALL(glDeleteTextures(1, &m_ColorAttachment));
            EVA_GL_CALL(glDeleteTextures(1, &m_DepthAttachment));
        }

        EVA_GL_CALL(glCreateFramebuffers(1, &m_RendererId));
        EVA_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));

        EVA_GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment));
        EVA_GL_CALL(glBindTexture(GL_TEXTURE_2D, m_ColorAttachment));
        EVA_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, OpenGLTexture::GetGLFormat(m_Specification.format), m_Specification.width,
                                 m_Specification.height, 0, OpenGLTexture::GetGLFormat(GetTextureFormat(m_Specification.format)),
                                 OpenGLTexture::GetGLDataType(GetTextureDataType(m_Specification.format)), nullptr));
        EVA_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        EVA_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        EVA_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0));

        EVA_GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment));
        EVA_GL_CALL(glBindTexture(GL_TEXTURE_2D, m_DepthAttachment));
        EVA_GL_CALL(glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Specification.width, m_Specification.height));
        EVA_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0));

        EVA_INTERNAL_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

        EVA_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void OpenGLFramebuffer::Bind()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));
        EVA_GL_CALL(glViewport(0, 0, m_Specification.width, m_Specification.height));
    }

    void OpenGLFramebuffer::Unbind()
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

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
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));
        EVA_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0));
        EVA_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0));
    }

    void OpenGLFramebuffer::AttachTexture(std::shared_ptr<Texture> texture, int mip)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));
        EVA_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (GLenum)texture->GetTarget(), texture->GetRendererId(), mip));
    }

    void OpenGLFramebuffer::AttachCubemap(std::shared_ptr<Texture> cubemap, int sideIndex, int mip)
    {
        EVA_PROFILE_FUNCTION();
        EVA_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));
        EVA_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + sideIndex,
                                           cubemap->GetRendererId(), mip));
    }
} // namespace EVA
