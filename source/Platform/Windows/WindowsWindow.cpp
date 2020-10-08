#include "WindowsWindow.hpp"

#include "EVA/Core.hpp"

namespace EVA
{
    static bool s_GLFWInitialized = false;
    std::unique_ptr<Window> Window::Create(const WindowProperties &properties)
    {
        return std::make_unique<WindowsWindow>(properties);
    }

    WindowsWindow::WindowsWindow(const WindowProperties &properties)
    {
        Init(properties);
    }

    WindowsWindow::~WindowsWindow()
    {
        Shutdown();
    }

    void WindowsWindow::Init(const WindowProperties &properties)
    {
        m_Data.title = properties.title;
        m_Data.width = properties.width;
        m_Data.height = properties.height;

        EVA_INTERNAL_INFO("Creating window {0} ({1}x{2})", m_Data.title, m_Data.width, m_Data.height);

        if(!s_GLFWInitialized)
        {
            int success = glfwInit();
            EVA_INTERNAL_ASSERT(success, "Could not initialize GLFW");
            s_GLFWInitialized = true;
        }

        m_Window = glfwCreateWindow(m_Data.width, m_Data.height, m_Data.title.c_str(), nullptr, nullptr);

        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, &m_Data);

        SetVSync(true);
    }

    void WindowsWindow::Shutdown()
    {
        glfwDestroyWindow(m_Window);
    }

    void WindowsWindow::OnUpdate()
    {
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
    }

    void WindowsWindow::SetVSync(bool enabled)
    {
        if(enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_Data.vSync = enabled;
    }
}
