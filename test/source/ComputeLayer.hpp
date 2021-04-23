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
        Ref<Framebuffer> m_ViewportFramebuffer;

        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        bool m_ResizeViewport  = false;

        EVA::Ref<EVA::Texture> m_ComputeTexture;
        EVA::Ref<EVA::Shader> m_ComputeShader;
        EVA::Ref<EVA::ShaderStorageBuffer> m_Ssbo;
        std::vector<glm::vec4> m_SsboData;
        size_t m_MaxObjects = 100;

        Ref<Mesh> m_ShipMesh;
        Ref<Shader> m_PBRShader;

        Ref<Texture> m_EquirectangularMap;
        Ref<Texture> m_EnvironmentMap;
        Ref<Texture> m_IrradianceMap;
        Ref<Texture> m_PreFilterMap;
        Ref<Texture> m_PreComputedBRDF;

        PerspectiveCameraController m_CameraController;


      public:
        ComputeLayer() :
          Layer("Compute"),
          m_FrameTimes(10),
          m_CameraController(glm::vec3(0, 2, -5), 0, 0, (float) Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight())
        {
            EVA_PROFILE_FUNCTION();

            // Framebuffer
            FramebufferSpecification spec;
            spec.width            = Application::Get().GetWindow().GetWidth();
            spec.height           = Application::Get().GetWindow().GetHeight();
            m_ViewportFramebuffer = Framebuffer::Create(spec);

            // Compute
            m_ComputeShader  = Shader::Create("./assets/shaders/compute.glsl");
            m_ComputeTexture = TextureManager::CreateTexture(512, 512, TextureFormat::RGBA8);
            m_Ssbo = ShaderStorageBuffer::Create(m_SsboData.data(), m_SsboData.size() * sizeof(glm::vec4));
            
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_Ssbo->GetRendererId());

            // Ship
            auto mesh  = Mesh::LoadMesh("./assets/models/colonial_fighter_red_fox/colonial_fighter_red_fox.obj");
            m_ShipMesh = mesh[0];
            m_PBRShader = Shader::Create("./assets/shaders/pbr.glsl");

            // Enviroment
            m_EquirectangularMap = TextureManager::LoadTexture("./assets/textures/space_1k.hdr");
            m_EnvironmentMap  = TextureUtilities::EquirectangularToCubemap(m_EquirectangularMap);
            m_IrradianceMap   = TextureUtilities::ConvoluteCubemap(m_EnvironmentMap);
            m_PreFilterMap    = TextureUtilities::PreFilterEnviromentMap(m_EnvironmentMap);
            m_PreComputedBRDF = TextureUtilities::PreComputeBRDF();
        }

        inline static float timer = 0.0f;
        void OnUpdate() override
        {
            EVA_PROFILE_FUNCTION();

            auto dt = EVA::Platform::GetDeltaTime();
            m_FrameTimes.Add(dt);

            // Update
            if (m_ViewportFocused)
            {
                if (Input::IsKeyPressed(KeyCode::Escape))
                {
                    if (Input::GetCursorMode() == Input::CursorMode::Disabled)
                        Input::SetCursorMode(Input::CursorMode::Normal);
                    else
                        Input::SetCursorMode(Input::CursorMode::Disabled);
                }

                m_CameraController.OnUpdate();
            }
            else if (Input::GetCursorMode() != Input::CursorMode::Normal)
            {
                Input::SetCursorMode(Input::CursorMode::Normal);
            }
            
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
                m_ViewportFramebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
                m_ComputeTexture = EVA::TextureManager::CreateTexture(m_ViewportSize.x, m_ViewportSize.y, TextureFormat::RGBA8);
            }

            {
                EVA_PROFILE_SCOPE("Render");

                m_ViewportFramebuffer->Bind();
                RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 1});
                RenderCommand::Clear();

                Renderer::BeginScene(m_CameraController.GetCamera(), m_EnvironmentMap, m_IrradianceMap, m_PreFilterMap, m_PreComputedBRDF);

                m_PBRShader->Bind();
                m_PBRShader->ResetTextureUnit();
                m_ShipMesh->GetMaterial()->Bind(m_PBRShader);

                auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
                Renderer::Submit(m_PBRShader, m_ShipMesh->GetVertexArray(), transform);

                Renderer::EndScene();
                m_ViewportFramebuffer->Unbind();
            }

            {
                EVA_PROFILE_SCOPE("Render ray");

                size_t numPixels      = m_ComputeTexture->GetWidth() * m_ComputeTexture->GetHeight();
                size_t numObjects  = glm::min(m_SsboData.size(), m_MaxObjects);

                auto maxWorkGroupSize = OpenGLContext::MaxComputeWorkGroupSize();
                size_t workGroupSize  = glm::min(numPixels, (size_t)maxWorkGroupSize.x);
                size_t numWorkGroups  = (size_t)glm::ceil(numPixels / (float)workGroupSize);

                m_ComputeShader->Bind();
                m_ComputeShader->ResetTextureUnit();

                m_ComputeShader->SetUniformMat4("u_ViewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
                m_ComputeShader->SetUniformFloat("cameraFov", m_CameraController.GetFov());
                m_ComputeShader->SetUniformFloat("cameraNear", m_CameraController.GetNearPlane());
                m_ComputeShader->SetUniformFloat("cameraFar", m_CameraController.GetFarPlane());

                m_ComputeShader->SetUniformFloat("time", Platform::GetTime());
                m_ComputeShader->BindTexture("envMap", m_EquirectangularMap);
                m_ComputeShader->BindTexture("u_FbColor", TextureTarget::Texture2D, m_ViewportFramebuffer->GetColorAttachmentRendererId());
                m_ComputeShader->BindTexture("u_FbDepth", TextureTarget::Texture2D, m_ViewportFramebuffer->GetDepthAttachmentRendererId());

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
