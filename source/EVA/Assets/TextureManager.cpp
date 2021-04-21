#include "TextureManager.hpp"
#include "FileSystem.hpp"
#include "EVA/Renderer/Renderer.hpp"
#include "Platform/OpenGL/OpenGLTexture.hpp"
#include <stb_image.h>
namespace EVA
{
    TextureFormat GetFormat(int channels, bool isHDR)
    {
        if (isHDR) return TextureFormat::RGB16F;

        switch (channels)
        {
            case 1: return TextureFormat::R8;
            case 2: return TextureFormat::RG8;
            case 3: return TextureFormat::RGB8;
            case 4: return TextureFormat::RGBA8;
            default: EVA_INTERNAL_ASSERT(false, "Unknown format");
        }
    }

    Ref<Texture> TextureManager::LoadTexture(const std::filesystem::path& path, const TextureSettings& settings)
    {
        const auto pathString = FileSystem::ToString(path);

        // Return the id if the texture's already loaded
        auto it = s_Textures.find(pathString);
        if (it != s_Textures.end()) 
        { 
            if (auto ref = (*it).second.lock()) { return ref; }; 
        }

        EVA_INTERNAL_TRACE("TextureManager::LoadTexture - Loading texture: {}", pathString);

        // Load the image
        stbi_set_flip_vertically_on_load(true);

        int width, height, channels;
        void* data;
        bool isHDR = stbi_is_hdr(pathString.c_str());

        if (isHDR)
            data = stbi_loadf(pathString.c_str(), &width, &height, &channels, 0);
        else
            data = stbi_load(pathString.c_str(), &width, &height, &channels, 0);

        // If the image was loaded
        if (data)
        {
            auto texture      = CreateRef<Texture>();
            texture->m_Width  = width;
            texture->m_Height = height;
            texture->m_Format = GetFormat(channels, isHDR);
            texture->m_Settings = settings;
            texture->m_Target   = TextureTarget::Texture2D;
            texture->m_Path     = path;

            switch (Renderer::GetAPI())
            {
                case RendererAPI::API::None: break;
                case RendererAPI::API::OpenGL: texture->m_RendererId = OpenGLTexture::CreateGLTextureId(*texture, data, pathString); break;
                default: EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
            }

            // Cache
            s_Textures[pathString] = texture;

            stbi_image_free(data);
            
            EVA_INTERNAL_TRACE("TextureManager::LoadTexture - Texture id: {}", texture->GetRendererId());

            return texture;
        }
        else // If not
        {
            EVA_INTERNAL_TRACE("TextureManager::LoadTexture - Failed to load image: {}", pathString);
            return nullptr;
        }
    }

    Ref<Texture> TextureManager::CreateTexture(uint32_t width, uint32_t height, TextureFormat format, const TextureSettings& settings)
    {
        auto texture      = CreateRef<Texture>();
        texture->m_Width    = width;
        texture->m_Height   = height;
        texture->m_Format   = format;
        texture->m_Settings = settings;
        texture->m_Target   = TextureTarget::Texture2D;
        texture->m_Path     = "Internal";

        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: break;
            case RendererAPI::API::OpenGL: texture->m_RendererId = OpenGLTexture::CreateGLTextureId(*texture); break;
            default: EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
        }

        return texture;
    }

    void TextureManager::Delete(Texture& texture) 
    {
        const auto pathString = FileSystem::ToString(texture.GetPath());

        EVA_INTERNAL_TRACE("TextureManager::Delete - Deleting texture: {}", pathString);

        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None: break;
            case RendererAPI::API::OpenGL: OpenGLTexture::DeleteGLTexture(texture); break;
            default: EVA_INTERNAL_ASSERT(false, "Unknown RendererAPI");
        }

        const auto it = s_Textures.find(pathString);
        if (it != s_Textures.end()) { s_Textures.erase(it); }
    }

    Ref<RawTexture> TextureManager::LoadRaw(const std::filesystem::path& path)
    {
        const auto pathString = FileSystem::ToString(path);

        // No need to flip is as it's not used as a OpenGL texture
        stbi_set_flip_vertically_on_load(false);

        // Load the image data
        int width, height, channels;
        const auto data = stbi_load(pathString.c_str(), &width, &height, &channels, 0);

        // If the image was loaded
        if (data)
        {
            auto texture = CreateRef<RawTexture>();
            texture->data = data;
            texture->width = width;
            texture->height = height;
            texture->channels = channels;
            texture->path   = path;

            // Cache
            s_RawTextures[pathString] = texture;

            EVA_INTERNAL_TRACE("TextureManager::LoadRaw - Loaded image: {}", pathString);
            return texture;
        }
        else // If not
        {
            EVA_INTERNAL_TRACE("TextureManager::LoadRaw - Failed to load image: {}", pathString);
            return nullptr;
        }
    }

    void TextureManager::DeleteRaw(RawTexture& texture) 
    { 
        const auto pathString = FileSystem::ToString(texture.path);

        EVA_INTERNAL_TRACE("TextureManager::DeleteRaw - Deleting texture: {}", pathString);

        stbi_image_free(texture.data);

        const auto it = s_RawTextures.find(pathString);
        if (it != s_RawTextures.end()) { s_RawTextures.erase(it); }
    }
}