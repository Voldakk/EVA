#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EVA.hpp"
#include "EVA/Utility/SlidingWindow.hpp"

#include <glad/glad.h>

namespace EVA
{
    class EditorLayer : public Layer
    {
        Ref<VertexArray> m_TriangleVertexArray;
        Ref<VertexArray> m_SquareVertexArray;

        Ref<Shader> m_FlatColorShader, m_TextureShader;

        Ref<Texture> m_Texture;

        OrthographicCameraController m_OrtoCameraController;
        PerspectiveCameraController m_PersCameraController;

        SlidingWindow<float> m_FrameTimes;

        glm::vec3 m_SquareColor = glm::vec3(0.2f, 0.3f, 0.8f);

        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        Ref<Framebuffer> m_ViewportFramebuffer;

        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        bool m_ResizeViewport  = false;

        Ref<Mesh> m_CubeMesh;
        Ref<Mesh> m_ShipMesh;
        Ref<Shader> m_PBRShader;

        Ref<Texture> m_EnvironmentMap;
        Ref<Texture> m_IrradianceMap;
        Ref<Texture> m_PreFilterMap;
        Ref<Texture> m_PreComputedBRDF;

        Ref<Mesh> m_SkyboxMesh;
        Ref<Shader> m_SkyboxShader;

        std::vector<Light> m_Lights;

      public:
        EditorLayer() :
          Layer("Editor"),
          m_FrameTimes(10),
          m_OrtoCameraController((float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight()),
          m_PersCameraController(glm::vec3(0, 0, -5), 0, 0, (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight())
        {
            EVA_PROFILE_FUNCTION();

            LoadShaders();

            {
                // Triangle
                m_TriangleVertexArray = VertexArray::Create();

                // Vertex buffer
                float vertices[3 * 7] = {
                  -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f, 
                  0.5f, -0.5f, 0.0f, 0.2f, 0.3f,  0.8f,  1.0f, 
                  0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f,  1.0f,
                };
                Ref<VertexBuffer> vb = VertexBuffer::Create(vertices, sizeof(vertices));
                BufferLayout layout       = {{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float4, "a_Color"}};
                vb->SetLayout(layout);
                m_TriangleVertexArray->AddVertexBuffer(vb);

                // Index buffer
                uint32_t indices[3]           = {0, 1, 2};
                Ref<IndexBuffer> ib = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
                m_TriangleVertexArray->SetIndexBuffer(ib);
            }
            {
                // Square
                m_SquareVertexArray = VertexArray::Create();

                float vertices[5 * 4] = {
                    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 
                    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,
                    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 
                    -0.5f, 0.5f,  0.0f, 0.0f, 1.0f};
                Ref<VertexBuffer> vb = VertexBuffer::Create(vertices, sizeof(vertices));
                vb->SetLayout({{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float2, "a_TexCoord"}});
                m_SquareVertexArray->AddVertexBuffer(vb);

                uint32_t indices[6] = {
                  0, 1, 2, 2, 3, 0,
                };
                Ref<IndexBuffer> ib = IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
                m_SquareVertexArray->SetIndexBuffer(ib);
            }
            {
                m_CubeMesh = Mesh::LoadMesh("./assets/models/cube.obj")[0];
            }

            // Shaders
            m_FlatColorShader = Shader::Create("./assets/shaders/color.glsl");
            m_TextureShader   = Shader::Create("./assets/shaders/texture.glsl");

            // Texture
            m_Texture = TextureManager::LoadTexture("assets/textures/uv.png");

            // Framebuffer
            FramebufferSpecification spec;
            spec.width            = 1200;
            spec.height           = 600;
            m_ViewportFramebuffer = Framebuffer::Create(spec);

            // Ship
            auto mesh = Mesh::LoadMesh("./assets/models/colonial_fighter_red_fox/colonial_fighter_red_fox.obj");
            m_ShipMesh = mesh[0];

            {
                auto equirectangular = TextureManager::LoadTexture("./assets/textures/space_1k.hdr");
                //auto equirectangular = TextureManager::LoadTexture("./assets/textures/canyon.hdr");
                m_EnvironmentMap     = TextureUtilities::EquirectangularToCubemap(equirectangular);
                m_IrradianceMap   = TextureUtilities::ConvoluteCubemap(m_EnvironmentMap);
                m_PreFilterMap    = TextureUtilities::PreFilterEnviromentMap(m_EnvironmentMap);
                m_PreComputedBRDF = TextureUtilities::PreComputeBRDF();
            }

            m_SkyboxMesh = Mesh::LoadMesh("./assets/models/cube_inverted.obj")[0];
        }
        void LoadShaders()
        {
            m_SkyboxShader = Shader::Create("./assets/shaders/skybox.glsl");
            m_PBRShader    = Shader::Create("./assets/shaders/pbr.glsl");
        }

        inline static float timer = 0.0f;
        void OnUpdate() override
        {
            EVA_PROFILE_FUNCTION();

            auto dt = Platform::GetDeltaTime();
            m_FrameTimes.Add(dt);

            if (m_ViewportFocused) 
            { 
                if (Input::IsKeyPressed(KeyCode::Escape))
                {
                    if (Input::GetCursorMode() == Input::CursorMode::Disabled)
                        Input::SetCursorMode(Input::CursorMode::Normal);
                    else
                        Input::SetCursorMode(Input::CursorMode::Disabled);
                }

                m_OrtoCameraController.OnUpdate();
                m_PersCameraController.OnUpdate();
            }
            else if (Input::GetCursorMode() != Input::CursorMode::Normal)
            {
                Input::SetCursorMode(Input::CursorMode::Normal);
            }

            // Render
            if (m_ResizeViewport)
            {
                EVA_PROFILE_SCOPE("Resize viewport");
                m_ResizeViewport = false;
                m_ViewportFramebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
                m_OrtoCameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
                m_PersCameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
            }
            m_ViewportFramebuffer->Bind();
            RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
            RenderCommand::Clear();

            //Renderer::BeginScene(m_OrtoCameraController.GetCamera());
            Renderer::BeginScene(m_PersCameraController.GetCamera(), m_EnvironmentMap, m_IrradianceMap, m_PreFilterMap, m_PreComputedBRDF, m_Lights);

            // Sky
            m_SkyboxShader->Bind();
            m_SkyboxShader->ResetTextureUnit();
            RenderCommand::EnableDepth(false);
            Renderer::Submit(m_SkyboxShader, m_SkyboxMesh->GetVertexArray());
            RenderCommand::EnableDepth(true);

            // Squares
            m_FlatColorShader->Bind();
            m_FlatColorShader->SetUniformFloat3("u_Color", m_SquareColor);

            auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
            for (size_t y = 0; y < 20; y++)
            {
                for (size_t x = 0; x < 20; x++)
                {
                    auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(2 + x * 0.11f, y * 0.11f, 0.0f)) * scale;
                    Renderer::Submit(m_FlatColorShader, m_SquareVertexArray, transform);
                }
            }

            auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
            /*m_TextureShader->Bind();
            m_TextureShader->ResetTextureUnit();
            m_TextureShader->BindTexture("u_AlbedoMap", m_Texture);
            Renderer::Submit(m_TextureShader, m_SquareVertexArray, transform);*/

            // Cube
            m_TextureShader->Bind();
            m_TextureShader->ResetTextureUnit();
            m_TextureShader->BindTexture("u_AlbedoMap", m_Texture);
            Renderer::Submit(m_TextureShader, m_CubeMesh->GetVertexArray(), transform);

            // Ship
            m_PBRShader->Bind();
            m_PBRShader->ResetTextureUnit();
            m_ShipMesh->GetMaterial()->Bind(m_PBRShader);
            Renderer::Submit(m_PBRShader, m_ShipMesh->GetVertexArray());

            Renderer::EndScene();
            m_ViewportFramebuffer->Unbind();
        }

        void OnEvent(Event& e) override
        {
            m_OrtoCameraController.OnEvent(e);
            m_PersCameraController.OnEvent(e);
        }

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
            if (ImGui::Button("Reload shaders")) LoadShaders();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::ColorEdit3("Square color", glm::value_ptr(m_SquareColor));
            m_PersCameraController.Inspector();
            if (ImGui::Button("Add light")) m_Lights.push_back(Light());
            if (ImGui::Button("Remove light")) m_Lights.erase(m_Lights.end() - 1);
            for (auto& l : m_Lights)
            {
                l.Inspector();
                ImGui::Spacing();
                ImGui::Spacing();
            }
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
            //auto viewportTextureId = m_ShipMesh->GetMaterial()->metallic->GetRendererId();
            ImGui::Image(*reinterpret_cast<void**>(&viewportTextureId), viewportPanelSize, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});

            ImGui::End();
            ImGui::PopStyleVar(ImGuiStyleVar_WindowPadding);
        }
    };
} // namespace EVA
