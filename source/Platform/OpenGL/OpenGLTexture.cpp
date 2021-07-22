#include "OpenGLTexture.hpp"
#include "OpenGL.hpp"

namespace EVA
{
    uint32_t OpenGLTexture::CreateGLTextureId(const Texture& texture, const void* data, const std::string& id)
    {
        EVA_PROFILE_FUNCTION();

        auto internalformat = texture.GetFormat();
        auto format         = GetPixelDataFormat(internalformat);
        auto type           = GetTextureDataType(internalformat);

        uint32_t rendererId;
        EVA_GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &rendererId));
        EVA_GL_CALL(glTextureStorage2D(rendererId, 1, GetGLFormat(internalformat), texture.GetWidth(), texture.GetHeight()));

        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_WRAP_S, GetGLWrapping(texture.GetSettings().wrapping)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_WRAP_T, GetGLWrapping(texture.GetSettings().wrapping)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GetGLMinFilter(texture.GetSettings().minFilter)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GetGLMagFilter(texture.GetSettings().magFilter)));

        EVA_GL_CALL(glTextureSubImage2D(rendererId, 0, 0, 0, texture.GetWidth(), texture.GetHeight(), GetGLPixelDataFormat(format),
                                        GetGLDataType(type), data));

        if (id != "") 
        { 
            EVA_GL_CALL(glObjectLabel(GL_TEXTURE, rendererId, -1, id.c_str())); 
        }

        return rendererId;
    }

    uint32_t OpenGLTexture::CreateGLTextureId(const Texture& texture, const std::string& id)
    {
        EVA_PROFILE_FUNCTION();

        uint32_t rendererId;
        EVA_GL_CALL(glGenTextures(1, &rendererId));
        EVA_GL_CALL(glBindTexture(GL_TEXTURE_2D, rendererId));

        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_WRAP_S, GetGLWrapping(texture.GetSettings().wrapping)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_WRAP_T, GetGLWrapping(texture.GetSettings().wrapping)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GetGLMinFilter(texture.GetSettings().minFilter)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GetGLMagFilter(texture.GetSettings().magFilter)));

        auto internalformat = texture.GetFormat();
        auto format         = GetPixelDataFormat(internalformat);
        auto type           = GetTextureDataType(internalformat);

        //EVA_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, internalformat, texture.GetWidth(), texture.GetHeight(), 0, dataFormat, type, nullptr));
        EVA_GL_CALL(glTextureStorage2D(rendererId, 1, GetGLFormat(internalformat), texture.GetWidth(), texture.GetHeight()));

        if (id != "") { EVA_GL_CALL(glObjectLabel(GL_TEXTURE, rendererId, -1, id.c_str())); }

        EVA_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

        return rendererId;
    }

    uint32_t OpenGLTexture::CreateGLCubemapId(const Texture& texture, const std::string& id)
    {
        EVA_PROFILE_FUNCTION();

        uint32_t rendererId;
        EVA_GL_CALL(glGenTextures(1, &rendererId));
        EVA_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, rendererId));

        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_WRAP_S, GetGLWrapping(texture.GetSettings().wrapping)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_WRAP_T, GetGLWrapping(texture.GetSettings().wrapping)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_WRAP_R, GetGLWrapping(texture.GetSettings().wrapping)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GetGLMinFilter(texture.GetSettings().minFilter)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GetGLMagFilter(texture.GetSettings().magFilter)));

        auto internalformat = texture.GetFormat();
        auto format         = GetPixelDataFormat(internalformat);
        auto type           = GetTextureDataType(internalformat);

        for (unsigned int i = 0; i < 6; ++i)
        {
            EVA_GL_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GetGLFormat(internalformat), texture.GetWidth(),
                                     texture.GetHeight(), 0, GetGLPixelDataFormat(format), GetGLDataType(type), nullptr));
        }


        if (id != "") { EVA_GL_CALL(glObjectLabel(GL_TEXTURE, rendererId, -1, id.c_str())); }

        return rendererId;
    }

    uint32_t OpenGLTexture::CopyTexture(const Texture& source, const Texture& texture, const std::string& id) 
    { 
        EVA_PROFILE_FUNCTION();

        auto internalformat = texture.GetFormat();
        auto format         = GetPixelDataFormat(internalformat);
        auto type           = GetTextureDataType(internalformat);

        uint32_t rendererId;
        EVA_GL_CALL(glCreateTextures(GL_TEXTURE_2D, 1, &rendererId));
        EVA_GL_CALL(glTextureStorage2D(rendererId, 1, GetGLFormat(internalformat), texture.GetWidth(), texture.GetHeight()));

        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_WRAP_S, GetGLWrapping(texture.GetSettings().wrapping)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_WRAP_T, GetGLWrapping(texture.GetSettings().wrapping)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_MIN_FILTER, GetGLMinFilter(texture.GetSettings().minFilter)));
        EVA_GL_CALL(glTextureParameteri(rendererId, GL_TEXTURE_MAG_FILTER, GetGLMagFilter(texture.GetSettings().magFilter)));

        EVA_GL_CALL(glCopyImageSubData(source.GetRendererId(), GetGLTarget(source.GetTarget()), 0, 0, 0, 0, rendererId,
                           GetGLTarget(texture.GetTarget()), 0, 0, 0, 0, source.GetWidth(), source.GetHeight(), 1));

        if (id != "") { EVA_GL_CALL(glObjectLabel(GL_TEXTURE, rendererId, -1, id.c_str())); }

        return rendererId;
    }

    void OpenGLTexture::GetDataFromGpu(const Texture& texture, void* buffer, uint32_t bufferSize, int level) 
    { 
        EVA_PROFILE_FUNCTION();

        auto internalformat = texture.GetFormat();
        auto format         = GetPixelDataFormat(internalformat);
        auto type           = GetTextureDataType(internalformat);

        glGetTextureImage(texture.GetRendererId(), level, GetGLPixelDataFormat(format), GetGLDataType(type), bufferSize, buffer);
    }

    void OpenGLTexture::DeleteGLTexture(const Texture& texture)
    {
        EVA_PROFILE_FUNCTION();

        auto rendererId = texture.GetRendererId();
        EVA_GL_CALL(glDeleteTextures(1, &rendererId));
    }

    void OpenGLTexture::GenerateMipMaps(const Texture& texture)
    {
        EVA_PROFILE_FUNCTION();

        EVA_GL_CALL(glBindTexture(GetGLTarget(texture.GetTarget()), texture.GetRendererId()));
        EVA_GL_CALL(glGenerateMipmap(GetGLTarget(texture.GetTarget())));
    }

    GLenum OpenGLTexture::GetGLTarget(const TextureTarget value)
    {
        switch (value)
        {
            case EVA::TextureTarget::Texture1D: return GL_TEXTURE_1D;
            case EVA::TextureTarget::Texture2D: return GL_TEXTURE_2D;
            case EVA::TextureTarget::Texture3D: return GL_TEXTURE_3D;
            case EVA::TextureTarget::Texture1DArray: return GL_TEXTURE_1D_ARRAY;
            case EVA::TextureTarget::Texture2DArray: return GL_TEXTURE_2D_ARRAY;
            case EVA::TextureTarget::TextureRectangle: return GL_TEXTURE_RECTANGLE;
            case EVA::TextureTarget::TextureCubeMap: return GL_TEXTURE_CUBE_MAP;
            case EVA::TextureTarget::TextureCubeMapArray: return GL_TEXTURE_CUBE_MAP_ARRAY;
            case EVA::TextureTarget::TextureBuffer: return GL_TEXTURE_BUFFER;
            case EVA::TextureTarget::Texture2DMultisample: return GL_TEXTURE_2D_MULTISAMPLE;
            case EVA::TextureTarget::Texture2DMultisampleArray: return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
        }
        throw;
    }

    GLenum OpenGLTexture::GetGLDataType(const TextureDataType dataType)
    {
        switch (dataType)
        {
            case EVA::TextureDataType::UnsignedByte: return GL_UNSIGNED_BYTE;
            case EVA::TextureDataType::Byte: return GL_BYTE;
            case EVA::TextureDataType::UnsignedShort: return GL_UNSIGNED_SHORT;
            case EVA::TextureDataType::Short: return GL_SHORT;
            case EVA::TextureDataType::UnsignedInt: return GL_UNSIGNED_INT;
            case EVA::TextureDataType::Int: return GL_INT;
            case EVA::TextureDataType::HalfFloat: return GL_HALF_FLOAT;
            case EVA::TextureDataType::Float: return GL_FLOAT;
            case EVA::TextureDataType::UnsignedByte332: return GL_UNSIGNED_BYTE_3_3_2;
            case EVA::TextureDataType::UnsignedByte233Rev: return GL_UNSIGNED_BYTE_2_3_3_REV;
            case EVA::TextureDataType::UnsignedShort565: return GL_UNSIGNED_SHORT_5_6_5;
            case EVA::TextureDataType::UnsignedShort565Rev: return GL_UNSIGNED_SHORT_5_6_5_REV;
            case EVA::TextureDataType::UnsignedShort4444: return GL_UNSIGNED_SHORT_4_4_4_4;
            case EVA::TextureDataType::UnsignedShort4444Rev: return GL_UNSIGNED_SHORT_4_4_4_4_REV;
            case EVA::TextureDataType::UnsignedShort5551: return GL_UNSIGNED_SHORT_5_5_5_1;
            case EVA::TextureDataType::UnsignedShort1555Rev: return GL_UNSIGNED_SHORT_1_5_5_5_REV;
            case EVA::TextureDataType::UnsignedInt8888: return GL_UNSIGNED_INT_8_8_8_8;
            case EVA::TextureDataType::UnsignedInt8888Rev: return GL_UNSIGNED_INT_8_8_8_8_REV;
            case EVA::TextureDataType::UnsignedInt1010102: return GL_UNSIGNED_INT_10_10_10_2;
            case EVA::TextureDataType::UnsignedInt2101010Rev: return GL_UNSIGNED_INT_2_10_10_10_REV;
        }
        throw;
    }

        GLenum OpenGLTexture::GetGLPixelDataFormat(const PixelDataFormat value)
    {
        switch (value)
        {
            case EVA::PixelDataFormat::STENCIL_INDEX: return GL_STENCIL_INDEX;
            case EVA::PixelDataFormat::DEPTH_COMPONENT: return GL_DEPTH_COMPONENT;
            case EVA::PixelDataFormat::DEPTH_STENCIL: return GL_DEPTH_STENCIL;
            case EVA::PixelDataFormat::RED: return GL_RED;
            case EVA::PixelDataFormat::GREEN: return GL_GREEN;
            case EVA::PixelDataFormat::BLUE: return GL_BLUE;
            case EVA::PixelDataFormat::RG: return GL_RG;
            case EVA::PixelDataFormat::RGB: return GL_RGB;
            case EVA::PixelDataFormat::RGBA: return GL_RGBA;
            case EVA::PixelDataFormat::BGR: return GL_BGR;
            case EVA::PixelDataFormat::BGRA: return GL_BGRA;
            case EVA::PixelDataFormat::RED_INTEGER: return GL_RED_INTEGER;
            case EVA::PixelDataFormat::GREEN_INTEGER: return GL_GREEN_INTEGER;
            case EVA::PixelDataFormat::BLUE_INTEGER: return GL_BLUE_INTEGER;
            case EVA::PixelDataFormat::RG_INTEGER: return GL_RG_INTEGER;
            case EVA::PixelDataFormat::RGB_INTEGER: return GL_RGB_INTEGER;
            case EVA::PixelDataFormat::RGBA_INTEGER: return GL_RGBA_INTEGER;
            case EVA::PixelDataFormat::BGR_INTEGER: return GL_BGR_INTEGER;
            case EVA::PixelDataFormat::BGRA_INTEGER: return GL_BGRA_INTEGER;
        }
        throw;
    }

    GLenum OpenGLTexture::GetGLFormat(const TextureFormat format)
    {
        switch (format)
        {
            case EVA::TextureFormat::R8: return GL_R8;
            case EVA::TextureFormat::R8_SNORM: return GL_R8_SNORM;
            case EVA::TextureFormat::R16: return GL_R16;
            case EVA::TextureFormat::R16_SNORM: return GL_R16_SNORM;
            case EVA::TextureFormat::RG8: return GL_RG8;
            case EVA::TextureFormat::RG8_SNORM: return GL_RG8_SNORM;
            case EVA::TextureFormat::RG16: return GL_RG16;
            case EVA::TextureFormat::RG16_SNORM: return GL_RG16_SNORM;
            case EVA::TextureFormat::R3_G3_B2: return GL_R3_G3_B2;
            case EVA::TextureFormat::RGB4: return GL_RGB4;
            case EVA::TextureFormat::RGB5: return GL_RGB5;
            case EVA::TextureFormat::RGB8: return GL_RGB8;
            case EVA::TextureFormat::RGB8_SNORM: return GL_RGB8_SNORM;
            case EVA::TextureFormat::RGB10: return GL_RGB10;
            case EVA::TextureFormat::RGB12: return GL_RGB12;
            case EVA::TextureFormat::RGB16_SNORM: return GL_RGB16_SNORM;
            case EVA::TextureFormat::RGBA2: return GL_RGBA2;
            case EVA::TextureFormat::RGBA4: return GL_RGBA4;
            case EVA::TextureFormat::RGB5_A1: return GL_RGB5_A1;
            case EVA::TextureFormat::RGBA8: return GL_RGBA8;
            case EVA::TextureFormat::RGBA8_SNORM: return GL_RGBA8_SNORM;
            case EVA::TextureFormat::RGB10_A2: return GL_RGB10_A2;
            case EVA::TextureFormat::RGB10_A2UI: return GL_RGB10_A2UI;
            case EVA::TextureFormat::RGBA12: return GL_RGBA12;
            case EVA::TextureFormat::RGBA16: return GL_RGBA16;
            case EVA::TextureFormat::SRGB8: return GL_SRGB8;
            case EVA::TextureFormat::SRGB8_ALPHA8: return GL_SRGB8_ALPHA8;
            case EVA::TextureFormat::R16F: return GL_R16F;
            case EVA::TextureFormat::RG16F: return GL_RG16F;
            case EVA::TextureFormat::RGB16F: return GL_RGB16F;
            case EVA::TextureFormat::RGBA16F: return GL_RGBA16F;
            case EVA::TextureFormat::R32F: return GL_R32F;
            case EVA::TextureFormat::RG32F: return GL_RG32F;
            case EVA::TextureFormat::RGB32F: return GL_RGB32F;
            case EVA::TextureFormat::RGBA32F: return GL_RGBA32F;
            case EVA::TextureFormat::R11F_G11F_B10F: return GL_R11F_G11F_B10F;
            case EVA::TextureFormat::RGB9_E5: return GL_RGB9_E5;
            case EVA::TextureFormat::R8I: return GL_R8I;
            case EVA::TextureFormat::R8UI: return GL_R8UI;
            case EVA::TextureFormat::R16I: return GL_R16I;
            case EVA::TextureFormat::R16UI: return GL_R16UI;
            case EVA::TextureFormat::R32I: return GL_R32I;
            case EVA::TextureFormat::R32UI: return GL_R32UI;
            case EVA::TextureFormat::RG8I: return GL_RG8I;
            case EVA::TextureFormat::RG8UI: return GL_RG8UI;
            case EVA::TextureFormat::RG16I: return GL_RG16I;
            case EVA::TextureFormat::RG16UI: return GL_RG16UI;
            case EVA::TextureFormat::RG32I: return GL_RG32I;
            case EVA::TextureFormat::RG32UI: return GL_RG32UI;
            case EVA::TextureFormat::RGB8I: return GL_RGB8I;
            case EVA::TextureFormat::RGB8UI: return GL_RGB8UI;
            case EVA::TextureFormat::RGB16I: return GL_RGB16I;
            case EVA::TextureFormat::RGB16UI: return GL_RGB16UI;
            case EVA::TextureFormat::RGB32I: return GL_RGB32I;
            case EVA::TextureFormat::RGB32UI: return GL_RGB32UI;
            case EVA::TextureFormat::RGBA8I: return GL_RGBA8I;
            case EVA::TextureFormat::RGBA8UI: return GL_RGBA8UI;
            case EVA::TextureFormat::RGBA16I: return GL_RGBA16I;
            case EVA::TextureFormat::RGBA16UI: return GL_RGBA16UI;
            case EVA::TextureFormat::RGBA32I: return GL_RGBA32I;
            case EVA::TextureFormat::RGBA32UI: return GL_RGBA32UI;
            case EVA::TextureFormat::RED: return GL_RED;
            case EVA::TextureFormat::RG: return GL_RG;
            case EVA::TextureFormat::RGB: return GL_RGB;
            case EVA::TextureFormat::RGBA: return GL_RGBA;
        }
        throw;
    }

    GLint OpenGLTexture::GetGLMinFilter(const TextureMinFilter value)
    {
        switch (value)
        {
            case EVA::TextureMinFilter::Nearest: return GL_NEAREST;
            case EVA::TextureMinFilter::Linear: return GL_LINEAR;
            case EVA::TextureMinFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
            case EVA::TextureMinFilter::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
            case EVA::TextureMinFilter::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
            case EVA::TextureMinFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
        }
        throw;
    }

    GLint OpenGLTexture::GetGLMagFilter(const TextureMagFilter value)
    {
        switch (value)
        {
            case EVA::TextureMagFilter::Nearest: return GL_NEAREST;
            case EVA::TextureMagFilter::Linear: return GL_LINEAR;
        }
        throw;
    }

    GLint OpenGLTexture::GetGLWrapping(const TextureWrapping value)
    {
        switch (value)
        {
            case EVA::TextureWrapping::ClampToEdge: return GL_CLAMP_TO_EDGE;
            case EVA::TextureWrapping::Repeat: return GL_REPEAT;
            case EVA::TextureWrapping::MirroredRepeat: return GL_MIRRORED_REPEAT;
        }
        throw;
    }
} // namespace EVA