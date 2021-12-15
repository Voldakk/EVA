#include "OpenGL.hpp"

namespace EVA::OpenGL
{
    unsigned int GetGLAccess(const Access value)
    {
        switch (value)
        {
            case EVA::Access::ReadOnly: return GL_READ_ONLY;
            case EVA::Access::WriteOnly: return GL_WRITE_ONLY;
            case EVA::Access::ReadWrite: return GL_READ_WRITE;
        }
        EVA_INTERNAL_ASSERT(false, "Unknown Access");
        return 0;
    }

    unsigned int GetGLUsage(const Usage value)
    {
        switch (value)
        {
            case EVA::Usage::AppModifiedOnceDeviceUsedOnce: return GL_STREAM_DRAW;
            case EVA::Usage::AppModifiedOnceDeviceUsedRepeatedly: return GL_STATIC_DRAW;
            case EVA::Usage::AppModifiedRepeatedlyDeviceUsedRepeatedly: return GL_DYNAMIC_DRAW;

            case EVA::Usage::DeviceModifiedOnceAppUsedOnce: return GL_STREAM_READ;
            case EVA::Usage::DeviceModifiedOnceAppUsedRepeatedly: return GL_STATIC_READ;
            case EVA::Usage::DeviceModifiedRepeatedlyAppUsedRepeatedly: return GL_DYNAMIC_READ;

            case EVA::Usage::DeviceModifiedOnceDeviceUsedOnce: return GL_STREAM_COPY;
            case EVA::Usage::DeviceModifiedOnceDeviceUsedRepeatedly: return GL_STATIC_COPY;
            case EVA::Usage::DeviceModifiedRepeatedlyDeviceUsedRepeatedly: return GL_DYNAMIC_COPY;
        }
        EVA_INTERNAL_ASSERT(false, "Unknown Usage");
        return 0;
    }
} // namespace EVA::OpenGL