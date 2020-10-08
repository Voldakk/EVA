#pragma once

#include "Log.hpp"

#ifdef EVA_DEBUG
	#ifdef EVA_PLATFORM_WINDOWS
		#define EVA_DEBUG_BREAK() __debugbreak()
	#elif EVA_PLATFORM_LINUX
		#include <signal.h>
		#define EVA_DEBUG_BREAK() raise(SIGTRAP)
	#endif // EVA_WINDOWS

	#define EVA_ASSERT(x, ...) { if(!(x)) { EVA_ERROR("Assertion Failed: {0}", __VA_ARGS__); EVA_DEBUG_BREAK(); } }
	#define EVA_INTERNAL_ASSERT(x, ...) { if(!(x)) { EVA_INTERNAL_ERROR("Assertion Failed: {0}", __VA_ARGS__); EVA_DEBUG_BREAK(); } }
#else
	#define EVA_ASSERT(x, ...)
	#define EVA_INTERNAL_ASSERT(x, ...)
#endif // EVA_DEBUG
