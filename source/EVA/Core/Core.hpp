#pragma once

#include "EVA/Core/Log.hpp"

#ifdef EVA_DEBUG
#    ifdef EVA_PLATFORM_WINDOWS
#        define EVA_DEBUG_BREAK() __debugbreak()
#    elif EVA_PLATFORM_LINUX
#        include <signal.h>
#        define EVA_DEBUG_BREAK() raise(SIGTRAP)
#    endif // EVA_WINDOWS

#    define EVA_ASSERT(x, ...)                                                                                                             \
        {                                                                                                                                  \
            if (!(x))                                                                                                                      \
            {                                                                                                                              \
                EVA_ERROR("Assertion Failed: {0}", __VA_ARGS__);                                                                           \
                EVA_DEBUG_BREAK();                                                                                                         \
            }                                                                                                                              \
        }
#    define EVA_INTERNAL_ASSERT(x, ...)                                                                                                    \
        {                                                                                                                                  \
            if (!(x))                                                                                                                      \
            {                                                                                                                              \
                EVA_INTERNAL_ERROR("Assertion Failed: {0}", __VA_ARGS__);                                                                  \
                EVA_DEBUG_BREAK();                                                                                                         \
            }                                                                                                                              \
        }
#else
#    define EVA_ASSERT(x, ...)
#    define EVA_INTERNAL_ASSERT(x, ...)
#endif // EVA_DEBUG

#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#define BIT(x) (1 << x)

namespace EVA
{
    template<typename T>
    using Scope = std::unique_ptr<T>;
    template<typename T, typename... Args>
    constexpr Scope<T> CreateScope(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;
    template<typename T, typename... Args>
    constexpr Ref<T> CreateRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
} // namespace EVA
