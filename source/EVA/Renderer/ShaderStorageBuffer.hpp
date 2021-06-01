#pragma once

#include "Common.hpp"

namespace EVA
{
    class ShaderStorageBuffer
    {
      public:
        virtual ~ShaderStorageBuffer() {};

        virtual void BufferData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

        virtual uint32_t GetRendererId() const = 0;

        virtual void* Map(Access access) const = 0;
        virtual void Unmap() const = 0;

        static Ref<ShaderStorageBuffer> Create(const void* data, uint32_t size, Usage usage = Usage::AppModifiedRepeatedlyDeviceUsedRepeatedly);
    };
} // namespace EVA
