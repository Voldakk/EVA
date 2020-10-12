#pragma once

#include "LayerStack.hpp"
#include "Window.hpp"
#include "Events\Window.hpp"

namespace EVA
{
    class Application
    {
        std::unique_ptr<Window> m_Window;
        bool m_Running = true;

        LayerStack m_LayerStack;

    public:

        Application();
        ~Application();

        void Run();
        void Exit();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

    private:
        void OnEvent(Event& event);
        bool OnWindowClosed(WindowCloseEvent& event);
    };
}
