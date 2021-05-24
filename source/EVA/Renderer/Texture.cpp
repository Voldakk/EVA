#include "Texture.hpp"

#include "EVA/Assets/TextureManager.hpp"

namespace EVA
{
    Texture::~Texture()
    {
        EVA_PROFILE_FUNCTION();
        TextureManager::Delete(*this);
    }

    RawTexture::~RawTexture()
    {
        EVA_PROFILE_FUNCTION();
        TextureManager::DeleteRaw(*this);
    }
} // namespace EVA