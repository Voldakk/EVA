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

    template<typename T>
    using WeakRef = std::weak_ptr<T>;
} // namespace EVA


#if EVA_DEBUG
// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#    if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#        define EVA_FUNC_SIG __PRETTY_FUNCTION__
#    elif defined(__DMC__) && (__DMC__ >= 0x810)
#        define EVA_FUNC_SIG __PRETTY_FUNCTION__
#    elif (defined(__FUNCSIG__) || (_MSC_VER))
#        define EVA_FUNC_SIG __FUNCSIG__
#    elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#        define EVA_FUNC_SIG __FUNCTION__
#    elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#        define EVA_FUNC_SIG __FUNC__
#    elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#        define EVA_FUNC_SIG __func__
#    elif defined(__cplusplus) && (__cplusplus >= 201103)
#        define EVA_FUNC_SIG __func__
#    else
#        define EVA_FUNC_SIG "EVA_FUNC_SIG unknown!"
#    endif
#else
#    define EVA_FUNC_SIG
#endif

#define EVA_PROFILER_TRACE         0
#define EVA_LOG_RENDERER_API_CALLS 0
