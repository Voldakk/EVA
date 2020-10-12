#pragma once

#ifdef EVA_DISABLE_LOGGING

#define EVA_LOG_INIT()

#define EVA_LOG_TRACE(...)   
#define EVA_LOG_INFO(...)    
#define EVA_LOG_WARN(...)    
#define EVA_LOG_ERROR(...)   
#define EVA_LOG_CRITICAL(...)

#define EVA_TRACE(...)   
#define EVA_INFO(...)    
#define EVA_WARN(...)    
#define EVA_ERROR(...)   
#define EVA_CRITICAL(...)

#else
#include <spdlog/fmt/ostr.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace EVA
{
	class Log
	{
		inline static std::shared_ptr<spdlog::logger> s_EngineLogger;
		inline static std::shared_ptr<spdlog::logger> s_AppLogger;

		inline static bool s_Init;

	public:

		inline static std::shared_ptr<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; };
		inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return s_AppLogger; };

		static void Init()
		{
			if (s_Init)
				return;
			s_Init = true;

			spdlog::set_pattern("%^[%T] %n %v%$");

			s_EngineLogger = spdlog::stderr_color_mt("EVA");
			s_EngineLogger->set_level(spdlog::level::trace);

			s_AppLogger = spdlog::stderr_color_mt("APP");
			s_AppLogger->set_level(spdlog::level::trace);

			s_EngineLogger->info("Initialized log");
		}
	};

	#define EVA_LOG_INIT               Log::Init

	#define EVA_INTERNAL_TRACE(...)    Log::GetEngineLogger()->trace(__VA_ARGS__)
	#define EVA_INTERNAL_INFO(...)     Log::GetEngineLogger()->info(__VA_ARGS__)
	#define EVA_INTERNAL_WARN(...)     Log::GetEngineLogger()->warn(__VA_ARGS__)
	#define EVA_INTERNAL_ERROR(...)    Log::GetEngineLogger()->error(__VA_ARGS__)
	#define EVA_INTERNAL_CRITICAL(...) Log::GetEngineLogger()->critical(__VA_ARGS__)
}

#define EVA_TRACE(...)    ::EVA::Log::GetAppLogger()->trace(__VA_ARGS__)
#define EVA_INFO(...)     ::EVA::Log::GetAppLogger()->info(__VA_ARGS__)
#define EVA_WARN(...)     ::EVA::Log::GetAppLogger()->warn(__VA_ARGS__)
#define EVA_ERROR(...)    ::EVA::Log::GetAppLogger()->error(__VA_ARGS__)
#define EVA_CRITICAL(...) ::EVA::Log::GetAppLogger()->critical(__VA_ARGS__)

#endif // EVA_DISABLE_LOGGING