#include "OpenGLContext.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace EVA
{
    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {}

    void OpenGLContext::Init()
    {
        EVA_PROFILE_FUNCTION();

        glfwMakeContextCurrent(m_WindowHandle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        EVA_INTERNAL_ASSERT(status, "Failed to initialize GLAD");

        EVA_INTERNAL_INFO("OpenGL info:");
        EVA_INTERNAL_INFO("- Vendor: {0}", glGetString(GL_VENDOR));
        EVA_INTERNAL_INFO("- Renderer: {0}", glGetString(GL_RENDERER));
        EVA_INTERNAL_INFO("- Version: {0}", glGetString(GL_VERSION));
    }

    void OpenGLContext::SwapBuffers()
    {
        EVA_PROFILE_FUNCTION();
        glfwSwapBuffers(m_WindowHandle);
    }
} // namespace EVA
