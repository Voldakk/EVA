#pragma once

#include <glad/glad.h>
#include "OpenGLCaller.hpp"
#include "EVA/Renderer/Common.hpp"

namespace EVA::OpenGL
{
    unsigned int GetGLAccess(const Access value);
    unsigned int GetGLUsage(const Usage value);
} // namespace EVA::OpenGL
