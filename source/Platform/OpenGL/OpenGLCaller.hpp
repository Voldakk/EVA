#pragma once

namespace EVA
{
#define GL_LOG_CALLS EVA_LOG_RENDERER_API_CALLS

#ifdef EVA_DEBUG
#    if GL_LOG_CALLS

#        define EVA_GL_CALL(glFunc)                                                                                                        \
            ::EVA::GLClearError();                                                                                                         \
            ::EVA::GLLogCall(#glFunc, __FILE__, __LINE__);                                                                                 \
            glFunc;                                                                                                                        \
            EVA_INTERNAL_ASSERT(::EVA::GLErrorLogCall(#glFunc, __FILE__, __LINE__), "OpenGL error");                                       \
            glFinish();                                                                                                                    \
            EVA_INTERNAL_ASSERT(::EVA::GLErrorLogCall(#glFunc, __FILE__, __LINE__), "OpenGL error")
#    else

#        define EVA_GL_CALL(glFunc)                                                                                                        \
            ::EVA::GLClearError();                                                                                                         \
            glFunc;                                                                                                                        \
            EVA_INTERNAL_ASSERT(::EVA::GLErrorLogCall(#glFunc, __FILE__, __LINE__), "OpenGL error");                                       \
            glFinish();                                                                                                                    \
            EVA_INTERNAL_ASSERT(::EVA::GLErrorLogCall(#glFunc, __FILE__, __LINE__), "OpenGL error")

#    endif // LOG_GL_CALLS
#else

#    define EVA_GL_CALL(glFunc) glFunc;

#endif //_DEBUG

    void GLClearError();
    bool GLErrorLogCall(const char* function, const char* file, int line);
    void GLLogCall(const char* function, const char* file, int line);

} // namespace EVA