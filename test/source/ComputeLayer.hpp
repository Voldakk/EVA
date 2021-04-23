#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EVA.hpp"
#include "EVA/Utility/SlidingWindow.hpp"
#include "Platform/OpenGL/OpenGLShader.hpp"
#include "Platform/OpenGL/OpenGLContext.hpp"

#include <glad/glad.h>
#include <random>

float randomFloat()
{
    static std::random_device rd;
    static std::mt19937 e2(rd());
    static std::uniform_real_distribution<float> dist(-5, 5);
    return dist(e2);
}

float randomRadius()
{
    static std::random_device rd;
    static std::mt19937 e2(rd());
    static std::uniform_real_distribution<float> dist(0.1, 0.5);
    return dist(e2);
}

namespace EVA
{
    class ComputeLayer : public EVA::Layer
    {
        EVA::SlidingWindow<float> m_FrameTimes;

        glm::vec2 m_ViewportSize = {0.0f, 0.0f};

        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        bool m_ResizeViewport  = false;

        EVA::Ref<EVA::Texture> m_ComputeTexture;
        EVA::Ref<EVA::Shader> m_ComputeShader;
        EVA::Ref<EVA::ShaderStorageBuffer> m_Ssbo;
        std::vector<glm::vec4> m_SsboData;
        size_t m_MaxObjects = 100;

        EVA::Ref<Texture> m_EnviromentTexture;

        PerspectiveCameraController m_CameraController;


      public:
        ComputeLayer() :
          Layer("Compute"),
          m_FrameTimes(10),
          m_CameraController(glm::vec3(0, 2, -5), 0, 0, (float) Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight())
        {
            EVA_PROFILE_FUNCTION();

            m_EnviromentTexture = TextureManager::LoadTexture("./assets/textures/space_1k.hdr");
            

            // Compute
            m_ComputeShader  = Shader::Create("./assets/shaders/compute.glsl");
            m_ComputeTexture = TextureManager::CreateTexture(512, 512, TextureFormat::RGBA8);
            m_Ssbo = ShaderStorageBuffer::Create(m_SsboData.data(), m_SsboData.size() * sizeof(glm::vec4));
            
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_Ssbo->GetRendererId());
        }

        inline static float timer = 0.0f;
        void OnUpdate() override
        {
            EVA_PROFILE_FUNCTION();

            auto dt = EVA::Platform::GetDeltaTime();
            m_FrameTimes.Add(dt);

            if (m_ViewportFocused) { m_CameraController.OnUpdate(); }

            // Update
            if (m_SsboData.size() < m_MaxObjects) { 
                m_SsboData.push_back(glm::vec4(randomFloat(), randomFloat(), randomFloat(), randomRadius()));
                m_Ssbo->BufferData(m_SsboData.data(), m_SsboData.size() * sizeof(glm::vec4));
            }
            else
            {
                m_SsboData.erase(m_SsboData.begin(), m_SsboData.begin() + 5);
                m_Ssbo->BufferData(m_SsboData.data(), m_SsboData.size() * sizeof(glm::vec4));
            }

            // Render
            if (m_ResizeViewport)
            {
                EVA_PROFILE_SCOPE("Resize viewport");
                m_ResizeViewport = false;
                m_ComputeTexture = EVA::TextureManager::CreateTexture(m_ViewportSize.x, m_ViewportSize.y, TextureFormat::RGBA8);
            }

            {
                EVA_PROFILE_SCOPE("Render");

                size_t numPixels      = m_ComputeTexture->GetWidth() * m_ComputeTexture->GetHeight();
                size_t numObjects  = glm::min(m_SsboData.size(), m_MaxObjects);

                auto maxWorkGroupSize = OpenGLContext::MaxComputeWorkGroupSize();
                size_t workGroupSize  = glm::min(numPixels, (size_t)maxWorkGroupSize.x);
                size_t numWorkGroups  = (size_t)glm::ceil(numPixels / (float)workGroupSize);

                m_ComputeShader->Bind();
                m_ComputeShader->ResetTextureUnit();

                m_ComputeShader->SetUniformFloat3("cameraPosition", m_CameraController.GetPosition());
                m_ComputeShader->SetUniformFloat3("cameraForward", m_CameraController.GetForward());
                m_ComputeShader->SetUniformFloat3("cameraUp", m_CameraController.GetUp());
                m_ComputeShader->SetUniformFloat3("cameraRight", m_CameraController.GetRight());
                m_ComputeShader->SetUniformFloat("cameraFov", m_CameraController.GetFov());

                m_ComputeShader->SetUniformFloat("time", Platform::GetTime());
                m_ComputeShader->BindTexture("envMap", m_EnviromentTexture);

                m_ComputeShader->SetUniformInt("objectBufferCount", numObjects);

                m_ComputeShader->BindImageTexture("imgOutput", m_ComputeTexture);
                m_ComputeShader->DispatchCompute(numWorkGroups, 1, 1, workGroupSize, 1, 1);
            }
        }

        void OnEvent(EVA::Event& e) override { }

        void OnImGuiRender() override
        {
            EVA_PROFILE_FUNCTION();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            auto avgFrameTime = m_FrameTimes.GetAverage();
            ImGui::Begin("Metrics");
            ImGui::Text("FPS: %.2f", 1.0f / avgFrameTime);
            ImGui::Text("Frame time: %.2f ms", avgFrameTime * 1000);
            ImGui::Text("Objects: %d / %d", m_SsboData.size(), m_MaxObjects);
            ImGui::End();

            ImGui::Begin("Settings");
            m_CameraController.Inspector();
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
            //auto viewportTextureId = m_EnviromentTexture->GetRendererId();
            ImGui::Image(*reinterpret_cast<void**>(&viewportTextureId), viewportPanelSize, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});

            ImGui::End();
            ImGui::PopStyleVar(ImGuiStyleVar_WindowPadding);
        }
    };
} // namespace EVA
