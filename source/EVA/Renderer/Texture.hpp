#pragma once

namespace EVA
{
    class Texture
    {
      public:
        virtual ~Texture() = default;

        virtual uint32_t GetWidth() const      = 0;
        virtual uint32_t GetHeight() const     = 0;
        virtual uint32_t GetRendererId() const = 0;
    };

    class Texture2D : public Texture
    {
      public:
        static Ref<Texture2D> Create(const std::string& path);
        static Ref<Texture2D> Create(const uint32_t width, const uint32_t height);

        virtual void Resize(const uint32_t width, const uint32_t height) = 0;
    };
} // namespace EVA
