#pragma once

#include "EVA/Renderer/RendererAPI.hpp"

namespace EVA
{
    class OpenGLRendererAPI : public RendererAPI
    {
      public:
        OpenGLRendererAPI() = default;

        void Init() override;

        void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

        void Clear() override;
        void SetClearColor(const glm::vec4& color) override;

        void SetCullMode(CullMode mode) override;

        void EnableDepth(bool value) override;

        void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
    };
} // namespace EVA
