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
    class Camera
    {
        glm::vec3 m_Position;
        glm::quat m_Orientation;

        glm::vec3 m_Forward;
        glm::vec3 m_Right;
        glm::vec3 m_Up;

        glm::vec2 m_MousePos;
        float m_MouseSensitivity = 0.3f;

        float m_Pitch = 0.0f;
        float m_Yaw   = 0.0f;
        float m_Fov   = 0.0f;

        void CalculateAxis() 
        {
            m_Forward = glm::normalize(m_Orientation * glm::vec3(0, 0, 1));
            m_Right   = glm::normalize(m_Orientation * glm::vec3(1, 0, 0));
            m_Up      = glm::normalize(m_Orientation * glm::vec3(0, 1, 0));
        }

      public:
        
        Camera(glm::vec3 position, float fov, float pitch, float yaw) : m_Position(position), m_Fov(fov), m_Pitch(pitch), m_Yaw(yaw)
        { 
            m_MousePos = Input::GetMousePosition();
            OnUpdate();
        }

        void OnUpdate() 
        { 
            // Movement
            glm::vec3 movement = glm::vec3(0.0);

            // Front
            if (Input::IsKeyPressed(KeyCode::W)) movement += m_Forward;

            // Back
            if (Input::IsKeyPressed(KeyCode::S)) movement -= m_Forward;

            // Right
            if (Input::IsKeyPressed(KeyCode::D)) movement += m_Right;

            // Left
            if (Input::IsKeyPressed(KeyCode::A)) movement -= m_Right;

            // Up
            if (Input::IsKeyPressed(KeyCode::Space)) movement += m_Up;

            // Down
            if (Input::IsKeyPressed(KeyCode::LeftShift)) movement -= m_Up;

            m_Position += movement * Platform::GetDeltaTime().GetSeconds();
            
            // Mouse
            auto mousePos = Input::GetMousePosition();
            auto mouseMovement = (mousePos - m_MousePos) * m_MouseSensitivity;
            m_MousePos    = mousePos;

            m_Pitch += mouseMovement.y * m_MouseSensitivity;
            m_Yaw += mouseMovement.x * m_MouseSensitivity;

            // Clamp
            m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
            if (m_Yaw < 0.0f)
                m_Yaw += 360.0f;
            else if (m_Yaw > 360.0f)
                m_Yaw -= 360.0f;

            m_Orientation = glm::angleAxis(glm::radians(m_Yaw), glm::vec3(0, 1, 0));
            CalculateAxis();
            m_Orientation = glm::angleAxis(glm::radians(m_Pitch), m_Right) * m_Orientation;
            CalculateAxis();
        }

        void Inspector() 
        { 
            ImGui::Text("Camera");
            ImGui::SliderFloat("FOV", &m_Fov, 1, 90);
            ImGui::SliderFloat("Mouse sensitivity", &m_MouseSensitivity, 0, 1);
        }

        glm::vec3 GetPosition() const { return m_Position; }
        glm::vec3 GetForward() const { return m_Forward; }
        glm::vec3 GetRight() const { return m_Right; }
        glm::vec3 GetUp() const { return m_Up; }
        float GetFov() const { return m_Fov; }
    };

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

        EVA::Ref<Camera> m_Camera;


      public:
        ComputeLayer() :
          Layer("Compute"),
          m_FrameTimes(10)
        {
            EVA_PROFILE_FUNCTION();

            m_EnviromentTexture = TextureManager::LoadTexture("./assets/textures/space_1k.hdr");

            m_Camera = CreateRef<Camera>(glm::vec3(0, 2, -5), 60, 0, 0);

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

            if (m_ViewportFocused) { m_Camera->OnUpdate(); }

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
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->ResetTextureUnit();

                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->SetUniformFloat3("cameraPosition", m_Camera->GetPosition());
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->SetUniformFloat3("cameraForward", m_Camera->GetForward());
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->SetUniformFloat3("cameraUp", m_Camera->GetUp());
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->SetUniformFloat3("cameraRight", m_Camera->GetRight());
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->SetUniformFloat("cameraFov", m_Camera->GetFov());

                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->SetUniformFloat("time", Platform::GetTime());
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->BindTexture("envMap", m_EnviromentTexture);

                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->SetUniformInt("objectBufferCount", numObjects);

                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->BindImageTexture("imgOutput", m_ComputeTexture);
                std::dynamic_pointer_cast<EVA::OpenGLShader>(m_ComputeShader)->DispatchCompute(numWorkGroups, 1, 1, workGroupSize, 1, 1);
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
            m_Camera->Inspector();
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
