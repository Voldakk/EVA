#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EVA.hpp"
#include "EVA/Utility/SlidingWindow.hpp"
#include "Platform/OpenGL/OpenGLShader.hpp"

#include <glad/glad.h>

namespace EVA
{
    class EditorLayer : public EVA::Layer
    {
        EVA::Ref<EVA::VertexArray> m_TriangleVertexArray;
        EVA::Ref<EVA::VertexArray> m_SquareVertexArray;

        EVA::Ref<EVA::Shader> m_FlatColorShader, m_TextureShader;

        EVA::Ref<EVA::Texture2D> m_Texture;

        EVA::OrthographicCameraController m_CameraController;

        EVA::SlidingWindow<float> m_FrameTimes;

        glm::vec3 m_SquareColor = glm::vec3(0.2f, 0.3f, 0.8f);

        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        EVA::Ref<EVA::Framebuffer> m_ViewportFramebuffer;

        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        bool m_ResizeViewport  = false;

      public:
        EditorLayer() :
          Layer("Editor"),
          m_FrameTimes(10),
          m_CameraController((float)EVA::Application::Get().GetWindow().GetWidth() / (float)EVA::Application::Get().GetWindow().GetHeight())
        {
            {
                // Triangle
                m_TriangleVertexArray = EVA::VertexArray::Create();

                // Vertex buffer
                float vertices[3 * 7] = {
                  -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f, 0.5f, -0.5f, 0.0f, 0.2f,
                  0.3f,  0.8f,  1.0f, 0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f,  1.0f,
                };
                EVA::Ref<EVA::VertexBuffer> vb = EVA::VertexBuffer::Create(vertices, sizeof(vertices));
                EVA::BufferLayout layout       = {{EVA::ShaderDataType::Float3, "a_Position"}, {EVA::ShaderDataType::Float4, "a_Color"}};
                vb->SetLayout(layout);
                m_TriangleVertexArray->AddVertexBuffer(vb);

                // Index buffer
                uint32_t indices[3]           = {0, 1, 2};
                EVA::Ref<EVA::IndexBuffer> ib = EVA::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
                m_TriangleVertexArray->SetIndexBuffer(ib);
            }
            {
                // Square
                m_SquareVertexArray = EVA::VertexArray::Create();

                float vertices[5 * 4]          = {-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,
                                         0.5f,  0.5f,  0.0f, 1.0f, 1.0f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f};
                EVA::Ref<EVA::VertexBuffer> vb = EVA::VertexBuffer::Create(vertices, sizeof(vertices));
                vb->SetLayout({{EVA::ShaderDataType::Float3, "a_Position"}, {EVA::ShaderDataType::Float2, "a_TexCoord"}});
                m_SquareVertexArray->AddVertexBuffer(vb);

                uint32_t indices[6] = {
                  0, 1, 2, 2, 3, 0,
                };
                EVA::Ref<EVA::IndexBuffer> ib = EVA::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
                m_SquareVertexArray->SetIndexBuffer(ib);
            }

            // Shaders
            m_FlatColorShader = EVA::Shader::Create("./assets/shaders/color.glsl");
            m_TextureShader   = EVA::Shader::Create("./assets/shaders/texture.glsl");

            // Texture
            m_Texture = EVA::Texture2D::Create("assets/textures/uv.png");

            // Framebuffer
            EVA::FramebufferSpecification spec;
            spec.width            = 1200;
            spec.height           = 600;
            m_ViewportFramebuffer = EVA::Framebuffer::Create(spec);
        }
        inline static float timer = 0.0f;
        void OnUpdate() override
        {
            auto dt = EVA::Platform::GetDeltaTime();
            m_FrameTimes.Add(dt);

            if (m_ViewportFocused) { m_CameraController.OnUpdate(); }

            // Render
            if (m_ResizeViewport)
            {
                m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
                m_ViewportFramebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            }
            m_ViewportFramebuffer->Bind();
            EVA::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
            EVA::RenderCommand::Clear();

            EVA::Renderer::BeginScene(m_CameraController.GetCamera());

            // Squares
            m_FlatColorShader->Bind();
            std::dynamic_pointer_cast<EVA::OpenGLShader>(m_FlatColorShader)->SetUniformFloat3("u_Color", m_SquareColor);

            auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
            for (size_t y = 0; y < 20; y++)
            {
                for (size_t x = 0; x < 20; x++)
                {
                    auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(x * 0.11f, y * 0.11f, 0.0f)) * scale;
                    EVA::Renderer::Submit(m_FlatColorShader, m_SquareVertexArray, transform);
                }
            }

            auto scale2 = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
            m_TextureShader->Bind();
            std::dynamic_pointer_cast<EVA::OpenGLShader>(m_TextureShader)->BindTexture("u_Texture", m_Texture);
            EVA::Renderer::Submit(m_TextureShader, m_SquareVertexArray, scale2);

            EVA::Renderer::EndScene();
            m_ViewportFramebuffer->Unbind();
        }

        void OnEvent(EVA::Event& e) override { m_CameraController.OnEvent(e); }

        void OnImGuiRender() override
        {
            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            auto avgFrameTime = m_FrameTimes.GetAverage();
            ImGui::Begin("Metrics");
            ImGui::Text("FPS: %.2f", 1.0f / avgFrameTime);
            ImGui::Text("Frame time: %.2f ms", avgFrameTime * 1000);
            ImGui::End();

            ImGui::Begin("Settings");
            ImGui::ColorEdit3("Square color", glm::value_ptr(m_SquareColor));
            ImGui::End();

            // Viewport
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 {0, 0});
            ImGui::Begin("Viewport");

            m_ViewportFocused = ImGui::IsWindowFocused();
            m_ViewportHovered = ImGui::IsWindowHovered();

            Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);

            auto viewportPanelSize = ImGui::GetContentRegionAvail();
            if (m_ViewportSize != *reinterpret_cast<glm::vec2*>(&viewportPanelSize) && viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
            {
                m_ViewportSize   = {viewportPanelSize.x, viewportPanelSize.y};
                m_ResizeViewport = true;
            }
            auto viewportTextureId = m_ViewportFramebuffer->GetColorAttachmentRendererId();
            ImGui::Image(*reinterpret_cast<void**>(&viewportTextureId), viewportPanelSize, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});

            ImGui::End();
            ImGui::PopStyleVar(ImGuiStyleVar_WindowPadding);
        }
    };
} // namespace EVA
