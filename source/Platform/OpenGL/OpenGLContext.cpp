#include "OpenGLContext.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace EVA
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle)
	{

	}

	void OpenGLContext::Init()
	{
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
		glfwSwapBuffers(m_WindowHandle);
	}
}
