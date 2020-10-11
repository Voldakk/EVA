#include "WindowsWindow.hpp"

#include "EVA/Core.hpp"
#include "EVA/Events/Key.hpp"
#include "EVA/Events/Mouse.hpp"
#include "EVA/Events/Window.hpp"

namespace EVA
{
    static bool s_GLFWInitialized = false;

    static void GLFWErrorCallback(int error, const char* description)
    {
        EVA_INTERNAL_ERROR("GLFW error ({0}): {1}", error, description);
    }

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

    void WindowsWindow::Init(const WindowProperties& properties)
    {
        m_Data.title = properties.title;
        m_Data.width = properties.width;
        m_Data.height = properties.height;

        EVA_INTERNAL_INFO("Creating window {0} ({1}x{2})", m_Data.title, m_Data.width, m_Data.height);

        if (!s_GLFWInitialized)
        {
            int success = glfwInit();
            EVA_INTERNAL_ASSERT(success, "Could not initialize GLFW");
            glfwSetErrorCallback(GLFWErrorCallback);
            s_GLFWInitialized = true;
        }

        m_Window = glfwCreateWindow(m_Data.width, m_Data.height, m_Data.title.c_str(), nullptr, nullptr);

        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, &m_Data);

        SetVSync(true);

        glfwSetWindowSizeCallback(m_Window, 
            [](GLFWwindow* window, int width, int height)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                data.width = width;
                data.height = height;

                WindowResizeEvent event(width, height);
                data.eventCallback(event);
            }
        );

        glfwSetWindowCloseCallback(m_Window, 
            [](GLFWwindow* window)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                WindowCloseEvent event;
                data.eventCallback(event);
            }
        );

        glfwSetKeyCallback(m_Window, 
            [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                switch (action)
                {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    data.eventCallback(event);
                    break;
                }
                default:
                    break;
                }
            }
        );

        glfwSetMouseButtonCallback(m_Window, 
            [](GLFWwindow* window, int button, int action, int mods)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                switch (action)
                {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.eventCallback(event);
                    break;
                }
                default:
                    break;
                }
            }
        );

        glfwSetScrollCallback(m_Window, 
            [](GLFWwindow* window, double xOffset, double yOffset)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                MouseScrolledEvent event((float)xOffset, (float)yOffset);
                data.eventCallback(event);
            }
        );

        glfwSetCursorPosCallback(m_Window,
            [](GLFWwindow* window, double xPosition, double yPosition)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                MouseMovedEvent event((float)xPosition, (float)yPosition);
                data.eventCallback(event);
            }
        );
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
