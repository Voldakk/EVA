#include "Application.hpp"

#include "Core.hpp"
#include "Events/Window.hpp"

namespace EVA
{
	Application::Application()
	{
		EVA_LOG_INIT();
		EVA_INTERNAL_TRACE("Initializing application");
		EVA_INTERNAL_INFO("Platform: {}", EVA_PLATFORM);
		EVA_INTERNAL_INFO("Architecture: {}", EVA_ARCHITECTURE);
		EVA_INTERNAL_INFO("Configuration: {}", EVA_CONFIGURATION);

		m_Window = Window::Create();
	}

	Application::~Application()
	{

	}

	void Application::Run()
    {
        while(m_Running)
        {
            m_Window->OnUpdate();
        }
    }

	void Application::Exit()
	{
		
	}
}
