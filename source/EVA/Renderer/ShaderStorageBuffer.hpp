#pragma once

namespace EVA
{
    class ShaderStorageBuffer
    {
      public:
        virtual ~ShaderStorageBuffer() {};

        virtual void BufferData(void* data, uint32_t size) = 0;

        virtual uint32_t GetRendererId() const = 0;

        static Ref<ShaderStorageBuffer> Create(void* data, uint32_t size);
    };
} // namespace EVA
