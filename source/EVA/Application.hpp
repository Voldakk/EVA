#pragma once

#include "Window.hpp"

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
    };
}
