#pragma once

#include "Log.hpp"

#ifdef EVA_DEBUG
	#ifdef EVA_PLATFORM_WINDOWS
		#define EVA_DEBUGBREAK() __debugbreak()
	#elif EVA_PLATFORM_LINUX
		#include <signal.h>
		#define EVA_DEBUGBREAK() raise(SIGTRAP)
	#endif // EVA_WINDOWS

	#define EVA_ASSERT(x, ...) { if(!(x)) { EVA_ERROR("Assertion Failed: {0}", __VA_ARGS__); EVA_DEBUGBREAK(); } }
	#define EVA_INTERNAL_ASSERT(x, ...) { if(!(x)) { EVA_INTERNAL_ERROR("Assertion Failed: {0}", __VA_ARGS__); EVA_DEBUGBREAK(); } }
#else
	#define EVA_ASSERT(x, ...)
	#define EVA_INTERNAL_ASSERT(x, ...)
#endif // EVA_DEBUG
