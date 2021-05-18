#include "OpenGL.hpp"

namespace EVA
{
    std::string CleanFile(const std::string& file)
    {
        auto path = std::filesystem::path(file);
        return path.filename().string();
    }

    void GLClearError()
    {
        while (glGetError() != GL_NO_ERROR) {}
    }
    bool GLErrorLogCall(const char* function, const char* file, const int line)
    {
        while (const auto error = glGetError())
        {
            EVA_INTERNAL_ERROR("[OpenGL Error] ({})\n{} ({}): {}", error, CleanFile(file), line, function);
            Log::GetEngineLogger()->flush();
            return false;
        }
        return true;
    }

    void GLLogCall(const char* function, const char* file, const int line)
    {
        EVA_INTERNAL_TRACE("[OpenGL call] {} ({}): {}", CleanFile(file), line, function);
        Log::GetEngineLogger()->flush();
    }
} // namespace EVA