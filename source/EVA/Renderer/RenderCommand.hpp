#pragma once

#include "RendererAPI.hpp"

namespace EVA
{
    class RenderCommand
    {
        static RendererAPI* s_RendererAPI;

      public:
        inline static void Init() { s_RendererAPI->Init(); }

        inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            s_RendererAPI->SetViewport(x, y, width, height);
        }

        inline static void Clear() { s_RendererAPI->Clear(); }
        inline static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }

        inline static void SetCullMode(CullMode mode) { s_RendererAPI->SetCullMode(mode); }

        inline static void EnableDepth(bool value) { s_RendererAPI->EnableDepth(value); }

        inline static void DrawIndexed(const Ref<VertexArray>& vertexArray) { s_RendererAPI->DrawIndexed(vertexArray); }
    };
} // namespace EVA
