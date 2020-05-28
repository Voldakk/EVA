#include "Application.hpp"

#include "Core.hpp"
#include "Events/Window.hpp"

namespace EVA
{
	Application::Application()
	{
		EVA_LOG_INIT();
		EVA_INTERNAL_TRACE("Init application");
		EVA_INTERNAL_INFO("Platform: {}", EVA_PLATFORM);
		EVA_INTERNAL_INFO("Architexture: {}", EVA_ARCHITECTURE);
		EVA_INTERNAL_INFO("Configuration: {}", EVA_CONFIGURATION);
		WindowResizeEvent e(1920, 1080);
		EVA_WARN("Event: {}", e);
		EVA_INTERNAL_ASSERT(false, "Test");
	}

	Application::~Application()
	{

	}

	void Application::Run()
    {
        
    }

	void Application::Exit()
	{
		
	}
}
