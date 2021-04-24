#include "Texture.hpp"

#include "EVA/Assets/TextureManager.hpp"

namespace EVA
{
    Texture::~Texture() { TextureManager::Delete(*this); }

    RawTexture::~RawTexture() { TextureManager::DeleteRaw(*this); }
} // namespace EVA