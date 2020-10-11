#pragma once

#include "Window.hpp"
#include "Events\Window.hpp"

namespace EVA
{
    class Application
    {
        std::unique_ptr<Window> m_Window;
        bool m_Running = true;

    public:

        Application();
        ~Application();

        void Run();
        void Exit();

    private:
        void OnEvent(Event& event);
        bool OnWindowClosed(WindowCloseEvent& event);
    };
}
