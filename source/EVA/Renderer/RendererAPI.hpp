#pragma once

#include "VertexArray.hpp"

namespace EVA
{
    enum class CullMode
    {
        None, Back, Front, Both
    };

    class RendererAPI
    {
      public:
        enum class API
        {
            None = 0,
            OpenGL
        };

      private:
        inline static API s_API = RendererAPI::API::OpenGL;

      public:
        virtual ~RendererAPI() = default;
        virtual void Init()    = 0;

        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

        virtual void Clear()                               = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;

        virtual void SetCullMode(CullMode mode) = 0;

        virtual void EnableDepth(bool value) = 0;

        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;

        inline static API GetAPI() { return s_API; }
    };
} // namespace EVA
