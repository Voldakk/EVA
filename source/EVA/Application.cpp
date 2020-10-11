#include "Application.hpp"

#include "Core.hpp"

namespace EVA
{
#define BIND_EVENT_FN(f) std::bind(&Application::f, this, std::placeholders::_1)

	Application::Application()
	{
		EVA_LOG_INIT();
		EVA_INTERNAL_TRACE("Initializing application");
		EVA_INTERNAL_INFO("Platform: {}", EVA_PLATFORM);
		EVA_INTERNAL_INFO("Architecture: {}", EVA_ARCHITECTURE);
		EVA_INTERNAL_INFO("Configuration: {}", EVA_CONFIGURATION);

		m_Window = Window::Create();
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
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

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClosed));
		EVA_INTERNAL_TRACE("{0}", event);
	}

	bool Application::OnWindowClosed(WindowCloseEvent& event)
	{
		m_Running = false;
		return true;
	}
}
