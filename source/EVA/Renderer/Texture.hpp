#pragma once

#include "EVA/Assets/Asset.hpp"
#include "EVA/Assets/ISerializeable.hpp"

namespace EVA
{
    enum class TextureAccess
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    enum class TextureTarget
    {
        Texture1D,
        Texture2D,
        Texture3D,
        Texture1DArray,
        Texture2DArray,
        TextureRectangle,
        TextureCubeMap,
        TextureCubeMapArray,
        TextureBuffer,
        Texture2DMultisample,
        Texture2DMultisampleArray,
    };

    enum class TextureWrapping
    {
        ClampToEdge,
        Repeat,
        MirroredRepeat,
    };

    enum class TextureMinFilter
    {
        Nearest,
        Linear,
        NearestMipmapNearest,
        LinearMipmapNearest,
        NearestMipmapLinear,
        LinearMipmapLinear,
    };

    enum class TextureMagFilter
    {
        Nearest,
        Linear,
    };

    enum class TextureDataType
    {
        UnsignedByte,
        Byte,
        UnsignedShort,
        Short,
        UnsignedInt,
        Int,
        HalfFloat,
        Float,
        UnsignedByte332,
        UnsignedByte233Rev,
        UnsignedShort565,
        UnsignedShort565Rev,
        UnsignedShort4444,
        UnsignedShort4444Rev,
        UnsignedShort5551,
        UnsignedShort1555Rev,
        UnsignedInt8888,
        UnsignedInt8888Rev,
        UnsignedInt1010102,
        UnsignedInt2101010Rev,
    };

    enum class TextureFormat
    {
        R8,
        R8_SNORM,
        R16,
        R16_SNORM,
        RG8,
        RG8_SNORM,
        RG16,
        RG16_SNORM,
        R3_G3_B2,
        RGB4,
        RGB5,
        RGB8,
        RGB8_SNORM,
        RGB10,
        RGB12,
        RGB16_SNORM,
        RGBA2,
        RGBA4,
        RGB5_A1,
        RGBA8,
        RGBA8_SNORM,
        RGB10_A2,
        RGB10_A2UI,
        RGBA12,
        RGBA16,
        SRGB8,
        SRGB8_ALPHA8,
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,
        R32F,
        RG32F,
        RGB32F,
        RGBA32F,
        R11F_G11F_B10F,
        RGB9_E5,
        R8I,
        R8UI,
        R16I,
        R16UI,
        R32I,
        R32UI,
        RG8I,
        RG8UI,
        RG16I,
        RG16UI,
        RG32I,
        RG32UI,
        RGB8I,
        RGB8UI,
        RGB16I,
        RGB16UI,
        RGB32I,
        RGB32UI,
        RGBA8I,
        RGBA8UI,
        RGBA16I,
        RGBA16UI,
        RGBA32I,
        RGBA32UI,
        RED,
        RG,
        RGB,
        RGBA,
    };

    struct TextureSettings : public ISerializeable
    {
        TextureWrapping wrapping   = TextureWrapping::Repeat;
        TextureMinFilter minFilter = TextureMinFilter::Linear;
        TextureMagFilter magFilter = TextureMagFilter::Linear;

        void Serialize(DataObject& data) override 
        {
            
        }
    };
    inline bool operator==(const TextureSettings& lhs, const TextureSettings& rhs)
    { 
        return lhs.wrapping == rhs.wrapping && lhs.minFilter == rhs.minFilter && lhs.magFilter == rhs.magFilter;
    }
    inline bool operator!=(const TextureSettings& lhs, const TextureSettings& rhs) { return !operator==(lhs, rhs); }

    inline static TextureSettings DefaultTextureSettings = {};

    class TextureManager;

    class Texture : public Asset
    {
        friend TextureManager;

      public:
        Texture() = default;
        ~Texture();

        inline const uint32_t GetRendererId() const { return m_RendererId; }
        inline const uint32_t GetWidth() const { return m_Width; }
        inline const uint32_t GetHeight() const { return m_Height; }
        inline const TextureTarget GetTarget() const { return m_Target; }
        inline const TextureFormat GetFormat() const { return m_Format; }
        inline const std::filesystem::path GetPath() const { return m_Path; }

        inline const glm::vec2 GetSize() const { return glm::vec2(m_Width, m_Height); }

        inline TextureSettings& GetSettings() { return m_Settings; }
        inline const TextureSettings& GetSettings() const { return m_Settings; }

      private:
        uint32_t m_RendererId {};
        uint32_t m_Width {};
        uint32_t m_Height {};
        TextureTarget m_Target {};
        TextureFormat m_Format {};
        TextureSettings m_Settings {};
        std::filesystem::path m_Path {};
    };

    struct RawTexture
    {
        uint8_t* data {};
        uint32_t width {};
        uint32_t height {};
        uint32_t channels {};
        std::filesystem::path path {};

        ~RawTexture();
    };

    template<typename T>
    class GridData
    {
      public:
        GridData() = default;
        GridData(uint32_t width, uint32_t height) : m_Width(width), m_Height(height), m_Data(width * height) {}
        GridData(uint32_t width, uint32_t height, const T& value) : m_Width(width), m_Height(height), m_Data(width * height, value) {}

        void Resize(uint32_t width, uint32_t height)
        {
            m_Width  = width;
            m_Height = height;
            m_Data.resize(width * height);
        }
        void Resize(uint32_t width, uint32_t height, const T& value)
        {
            m_Width  = width;
            m_Height = height;
            m_Data.resize(width * height, value);
        }

        T* operator[](size_t row) { return &m_Data[row * m_Width]; }
        const T* operator[](size_t row) const { return &m_Data[row * m_Width]; }

        T* Row(size_t row) { return &m_Data[row * m_Width]; }
        const T* Row(size_t row) const { return &m_Data[row * m_Width]; }

        void* Data() { return static_cast<void*>(m_Data.data()); }
        const void* Data() const { return static_cast<const void*>(m_Data.data()); }

        uint32_t Width() const { return m_Width; }
        uint32_t Height() const { return m_Height; }
        uint32_t Count() const { return m_Width * m_Height; }
        uint32_t Size() const { return m_Width * m_Height * sizeof(T); }

      private:
        std::vector<T> m_Data;
        uint32_t m_Width;
        uint32_t m_Height;
    };

    inline TextureFormat GetTextureFormat(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::RED:
            case TextureFormat::R8:
            case TextureFormat::R8_SNORM:
            case TextureFormat::R16:
            case TextureFormat::R16_SNORM:
            case TextureFormat::R16F:
            case TextureFormat::R32F:
            case TextureFormat::R8I:
            case TextureFormat::R8UI:
            case TextureFormat::R16I:
            case TextureFormat::R16UI:
            case TextureFormat::R32I:
            case TextureFormat::R32UI: return TextureFormat::RED;

            case TextureFormat::RG:
            case TextureFormat::RG8:
            case TextureFormat::RG8_SNORM:
            case TextureFormat::RG16:
            case TextureFormat::RG16_SNORM:
            case TextureFormat::RG16F:
            case TextureFormat::RG32F:
            case TextureFormat::RG8I:
            case TextureFormat::RG8UI:
            case TextureFormat::RG16I:
            case TextureFormat::RG16UI:
            case TextureFormat::RG32I:
            case TextureFormat::RG32UI: return TextureFormat::RG;

            case TextureFormat::RGB:
            case TextureFormat::R3_G3_B2:
            case TextureFormat::RGB4:
            case TextureFormat::RGB5:
            case TextureFormat::RGB8:
            case TextureFormat::RGB8_SNORM:
            case TextureFormat::RGB10:
            case TextureFormat::RGB12:
            case TextureFormat::RGB16_SNORM:
            case TextureFormat::RGBA2:
            case TextureFormat::RGBA4:
            case TextureFormat::SRGB8:
            case TextureFormat::RGB16F:
            case TextureFormat::RGB32F:
            case TextureFormat::R11F_G11F_B10F:
            case TextureFormat::RGB9_E5:
            case TextureFormat::RGB8I:
            case TextureFormat::RGB8UI:
            case TextureFormat::RGB16I:
            case TextureFormat::RGB16UI:
            case TextureFormat::RGB32I:
            case TextureFormat::RGB32UI: return TextureFormat::RGB;

            case TextureFormat::RGBA:
            case TextureFormat::RGB5_A1:
            case TextureFormat::RGBA8:
            case TextureFormat::RGBA8_SNORM:
            case TextureFormat::RGB10_A2:
            case TextureFormat::RGB10_A2UI:
            case TextureFormat::RGBA12:
            case TextureFormat::RGBA16:
            case TextureFormat::SRGB8_ALPHA8:
            case TextureFormat::RGBA16F:
            case TextureFormat::RGBA32F:
            case TextureFormat::RGBA8I:
            case TextureFormat::RGBA8UI:
            case TextureFormat::RGBA16I:
            case TextureFormat::RGBA16UI:
            case TextureFormat::RGBA32I:
            case TextureFormat::RGBA32UI: return TextureFormat::RGBA;

            default: throw;
        }
    }

    inline TextureDataType GetTextureDataType(TextureFormat format)
    {
        switch (format)
        {
            case TextureFormat::RED:
            case TextureFormat::RG:
            case TextureFormat::RGB:
            case TextureFormat::RGBA:
            case TextureFormat::R8:
            case TextureFormat::RG8:
            case TextureFormat::RGB8:
            case TextureFormat::RGBA8:
            case TextureFormat::R8UI:
            case TextureFormat::RG8UI:
            case TextureFormat::RGB8UI:
            case TextureFormat::RGBA8UI:
            case TextureFormat::SRGB8:
            case TextureFormat::SRGB8_ALPHA8: return TextureDataType::UnsignedByte;

            case TextureFormat::R8I:
            case TextureFormat::RG8I:
            case TextureFormat::RGB8I:
            case TextureFormat::RGBA8I:
            case TextureFormat::R8_SNORM:
            case TextureFormat::RG8_SNORM:
            case TextureFormat::RGB8_SNORM:
            case TextureFormat::RGBA8_SNORM: return TextureDataType::Byte;

            case TextureFormat::R16:
            case TextureFormat::RG16:
            case TextureFormat::RGB16_SNORM:
            case TextureFormat::RGBA16:
            case TextureFormat::R16UI:
            case TextureFormat::RG16UI:
            case TextureFormat::RGB16UI:
            case TextureFormat::RGBA16UI: return TextureDataType::UnsignedShort;

            case TextureFormat::R16I:
            case TextureFormat::RG16I:
            case TextureFormat::RGB16I:
            case TextureFormat::RGBA16I:
            case TextureFormat::R16_SNORM:
            case TextureFormat::RG16_SNORM: return TextureDataType::Short;

            case TextureFormat::R32I:
            case TextureFormat::RG32I:
            case TextureFormat::RGB32I:
            case TextureFormat::RGBA32I: return TextureDataType::Int;

            case TextureFormat::R32UI:
            case TextureFormat::RG32UI:
            case TextureFormat::RGB32UI:
            case TextureFormat::RGBA32UI: return TextureDataType::UnsignedInt;

            case TextureFormat::R16F:
            case TextureFormat::RG16F:
            case TextureFormat::RGB16F:
            case TextureFormat::RGBA16F:
                // return TextureDataType::HalfFloat;
            case TextureFormat::R32F:
            case TextureFormat::RG32F:
            case TextureFormat::RGB32F:
            case TextureFormat::RGBA32F: return TextureDataType::Float;

            case TextureFormat::R3_G3_B2: return TextureDataType::UnsignedByte332;

            case TextureFormat::RGB5_A1: return TextureDataType::UnsignedShort5551;

            case TextureFormat::RGB10_A2:
            case TextureFormat::RGB10_A2UI: return TextureDataType::UnsignedInt1010102;

            case TextureFormat::RGB4:
            case TextureFormat::RGB5:
            case TextureFormat::RGB10:
            case TextureFormat::RGB12:
            case TextureFormat::RGBA2:
            case TextureFormat::RGBA4:
            case TextureFormat::R11F_G11F_B10F:
            case TextureFormat::RGB9_E5:
            case TextureFormat::RGBA12:

            default: throw;
        }
    }

    inline uint32_t GetTextureChannels(TextureFormat format)
    {
        switch (GetTextureFormat(format))
        {
            case TextureFormat::RED: return 1;
            case TextureFormat::RG: return 2;
            case TextureFormat::RGB: return 3;
            case TextureFormat::RGBA: return 4;
        }
        EVA_INTERNAL_ASSERT(false, "Unknown texture format");
        return 0;
    }
} // namespace EVA
