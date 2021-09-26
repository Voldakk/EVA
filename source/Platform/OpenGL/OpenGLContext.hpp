#pragma once

#include <GLFW/glfw3.h>

#include "EVA/Renderer/GraphicsContext.hpp"

namespace EVA
{
    class OpenGLContext : public GraphicsContext
    {
        GLFWwindow* m_WindowHandle;

      public:
        OpenGLContext(GLFWwindow* windowHandle);
        virtual void Init() override;
        virtual void SwapBuffers() override;

        virtual uint32_t MaxUnifromBufferBindings() override;

        static glm::ivec3 MaxComputeWorkGroupSize();
        static int MaxComputeVariableWorkGroupSize();
    };
} // namespace EVA
