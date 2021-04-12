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

    glm::ivec3 OpenGLContext::MaxComputeWorkGroupSize()
    {
        static glm::ivec3 size {0, 0, 0};
        if (size.x > 0) return size;

        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &size[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &size[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &size[2]);

        EVA_INTERNAL_INFO("MaxComputeWorkGroupSize: {}, {}, {}", size[0], size[1], size[2]);

        return size;
    }
} // namespace EVA
