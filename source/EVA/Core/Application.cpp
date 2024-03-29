#include "Application.hpp"

#include "EVA/Core/Input.hpp"
#include "EVA/Core/Platform.hpp"
#include "EVA/Core/Timestep.hpp"
#include "EVA/Renderer/Renderer.hpp"

namespace EVA
{
    Application::Application(const WindowProperties& properties)
    {
        EVA_INTERNAL_ASSERT(s_Instance == nullptr, "Application already exists");
        s_Instance = this;
        EVA_LOG_INIT();

        EVA_PROFILE_FUNCTION();

        EVA_INTERNAL_INFO("Platform: {}", EVA_PLATFORM);
        EVA_INTERNAL_INFO("Architecture: {}", EVA_ARCHITECTURE);
        EVA_INTERNAL_INFO("Configuration: {}", EVA_CONFIGURATION);

        m_Window = Window::Create(properties);
        m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);

        EVA_INTERNAL_TRACE("Initialized application");
    }

    Application::~Application() { EVA_PROFILE_FUNCTION(); }

    void Application::Run()
    {
        EVA_PROFILE_FUNCTION();

        float lastFrameTime = Platform::GetTime();

        while (m_Running)
        {
            EVA_PROFILE_SCOPE("Run loop");

            float time = Platform::GetTime();
            Platform::SetDeltaTime(time - lastFrameTime);
            lastFrameTime = time;

            if (!m_Minimized)
            {
                for (auto layer : m_LayerStack)
                    layer->OnUpdate();
            }

            m_ImGuiLayer->Begin();
            for (auto layer : m_LayerStack)
                layer->OnImGuiRender();
            m_ImGuiLayer->End();

            m_Window->OnUpdate();
        }
    }

    void Application::Exit() { m_Running = false; }

    void Application::PushLayer(Layer* layer)
    {
        EVA_PROFILE_FUNCTION();

        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* overlay)
    {
        EVA_PROFILE_FUNCTION();

        m_LayerStack.PushOverlay(overlay);
        overlay->OnAttach();
    }

    void Application::OnEvent(Event& event)
    {
        EVA_PROFILE_FUNCTION();

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClosed));
        dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResized));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
        {
            (*--it)->OnEvent(event);
            if (event.handled) break;
        }
    }

    bool Application::OnWindowClosed(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResized(WindowResizeEvent& e)
    {
        EVA_PROFILE_FUNCTION();

        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        return false;
    }
} // namespace EVA
