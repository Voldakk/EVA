#include "OpenGLContext.hpp"

#include <GLFW/glfw3.h>
#include "OpenGL.hpp"

std::string GLMessageTypeToString(GLenum type)
{
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR: return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
        case GL_DEBUG_TYPE_MARKER: return "MARKER";
        case GL_DEBUG_TYPE_PUSH_GROUP: return "PUSH_GROUP";
        case GL_DEBUG_TYPE_POP_GROUP: return "POP_GROUP";
        case GL_DEBUG_TYPE_OTHER: return "OTHER";
        default: return "Unknown";
    }
}

std::string GLMessageSourceToString(GLenum type)
{
    switch (type)
    {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW_SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER_COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD_PARTY";
        case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER: return "OTHER";
        default: return "Unknown";
    }
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    std::string m(message);
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
        {
            EVA_INTERNAL_ERROR("GL CALLBACK - {} - {}\n{}", GLMessageSourceToString(source),
                               GLMessageTypeToString(type), m);
        }
        break;
        case GL_DEBUG_SEVERITY_MEDIUM:
        {
            EVA_INTERNAL_WARN("GL CALLBACK - {} - {}\n{}", GLMessageSourceToString(source),
                              GLMessageTypeToString(type), m);
        }
        break;
        case GL_DEBUG_SEVERITY_LOW:
        {
            EVA_INTERNAL_INFO("GL CALLBACK - {} - {}\n{}", GLMessageSourceToString(source),
                               GLMessageTypeToString(type), m);
        }
        break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
        default:
        {
            EVA_INTERNAL_TRACE("GL CALLBACK - {} - {}\n{}", GLMessageSourceToString(source),
                               GLMessageTypeToString(type), m);
        }
        break;
    }
}

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

#ifdef EVA_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
#endif // EVA_DEBUG
    }

    void OpenGLContext::SwapBuffers()
    {
        EVA_PROFILE_FUNCTION();
        glfwSwapBuffers(m_WindowHandle);
    }

    glm::ivec3 OpenGLContext::MaxComputeWorkGroupSize()
    {
        EVA_PROFILE_FUNCTION();

        static glm::ivec3 size {0, 0, 0};
        if (size.x > 0) { return size; };

        EVA_GL_CALL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &size[0]));
        EVA_GL_CALL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &size[1]));
        EVA_GL_CALL(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &size[2]));

        EVA_INTERNAL_INFO("MaxComputeWorkGroupSize: {}, {}, {}", size[0], size[1], size[2]);

        return size;
    }

    GLint OpenGLContext::MaxComputeVariableWorkGroupSize()
    {
        EVA_PROFILE_FUNCTION();

        static GLint size {0};
        if (size > 0) { return size; };

        EVA_GL_CALL(glGetIntegerv(GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB, &size));

        EVA_INTERNAL_INFO("MaxComputeVariableWorkGroupSize: {}", size);

        return size;
    }
} // namespace EVA
