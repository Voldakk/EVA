#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EVA.hpp"
#include "EVA/Utility/SlidingWindow.hpp"
#include "Platform/OpenGL/OpenGLShader.hpp"

#include <glad/glad.h>

namespace EVA
{
    class ComputeLayer : public EVA::Layer
    {
       EVA::OrthographicCameraController m_CameraController;

        EVA::SlidingWindow<float> m_FrameTimes;

        glm::vec2 m_ViewportSize = {0.0f, 0.0f};

        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        bool m_ResizeViewport  = false;

        EVA::Ref<EVA::Texture2D> m_ComputeTexture;
        EVA::Ref<EVA::Shader> m_ComputeShader;
        EVA::Ref<EVA::ShaderStorageBuffer> m_Ssbo;
        std::vector<glm::vec2> ssboData;

      public:
        ComputeLayer() :
          Layer("Compute"),
          m_FrameTimes(10),
          m_CameraController((float)EVA::Application::Get().GetWindow().GetWidth() / (float)EVA::Application::Get().GetWindow().GetHeight())
        {
            EVA_PROFILE_FUNCTION();

            // Compute
            m_ComputeShader  = EVA::Shader::Create("./assets/shaders/compute.glsl");
            m_ComputeTexture = EVA::Texture2D::Create(512, 512);

            while (ssboData.size() < 200)
            {
                ssboData.push_back(glm::vec2(200 - rand() % 400, 200 - rand() % 400));
            }
            m_Ssbo = EVA::ShaderStorageBuffer::Create(ssboData.data(), ssboData.size() * sizeof(glm::vec2));
            
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_Ssbo->GetRendererId());
        }

        inline static float timer = 0.0f;
        void OnUpdate() override
        {
            EVA_PROFILE_FUNCTION();

            auto dt = EVA::Platform::GetDeltaTime();
            m_FrameTimes.Add(dt);

            if (m_ViewportFocused) { m_CameraController.OnUpdate(); }

            // Render
            if (m_ResizeViewport)
            {
                EVA_PROFILE_SCOPE("Resize viewport");
                m_ResizeViewport = false;
                m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
                m_ComputeTexture->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            }

            {
                EVA_PROFILE_SCOPE("Render");

                auto start = std::chrono::steady_clock::now();

                m_ComputeShader->Bind();
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->SetUniformInt("objectBufferCount", ssboData.size());
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->ResetTextureUnit();
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->BindImageTexture("imgOutput", m_ComputeTexture);
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)
                  ->DispatchCompute(m_ComputeTexture->GetWidth(), m_ComputeTexture->GetHeight(), 1);
            }
        }

        void OnEvent(EVA::Event& e) override { m_CameraController.OnEvent(e); }

        void OnImGuiRender() override
        {
            EVA_PROFILE_FUNCTION();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            auto avgFrameTime = m_FrameTimes.GetAverage();
            ImGui::Begin("Metrics");
            ImGui::Text("FPS: %.2f", 1.0f / avgFrameTime);
            ImGui::Text("Frame time: %.2f ms", avgFrameTime * 1000);
            ImGui::End();

            ImGui::Begin("Settings");
            
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
            auto viewportTextureId = m_ComputeTexture->GetRendererId();
            ImGui::Image(*reinterpret_cast<void**>(&viewportTextureId), viewportPanelSize, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});

            ImGui::End();
            ImGui::PopStyleVar(ImGuiStyleVar_WindowPadding);
        }
    };
} // namespace EVA
