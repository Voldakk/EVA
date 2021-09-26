#pragma once

#include "EVA.hpp"
#include "EVA/Utility/SlidingWindow.hpp"
#include "Viewport.hpp"
#include <glad/glad.h>

namespace EVA
{
    class EditorLayer : public Layer
    {
        Ref<VertexArray> m_TriangleVertexArray;
        Ref<VertexArray> m_SquareVertexArray;

        Ref<Shader> m_FlatColorShader, m_TextureShader;

        OrthographicCameraController m_OrtoCameraController;
        PerspectiveCameraController m_PersCameraController;

        SlidingWindow<float> m_FrameTimes;

        glm::vec3 m_SquareColor = glm::vec3(0.2f, 0.3f, 0.8f);

        Ref<Mesh> m_CubeMesh;
        Ref<Material> m_CubeMaterial;

        Ref<Mesh> m_ShipMesh;
        Ref<Shader> m_PBRShader;

        Ref<Viewport> m_Viewport;
        Ref<Environment> m_Environment;

        std::vector<Light> m_Lights;

      public:
        EditorLayer() :
          Layer("Editor"),
          m_FrameTimes(10),
          m_OrtoCameraController((float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight()),
          m_PersCameraController(glm::vec3(0, 0, -5), 0, 0,
                                 (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight())
        {
            EVA_PROFILE_FUNCTION();

            LoadShaders();

            {
                // Triangle
                m_TriangleVertexArray = VertexArray::Create();

                // Vertex buffer
                float vertices[3 * 7] = {
                  -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f, 0.5f, -0.5f, 0.0f, 0.2f,
                  0.3f,  0.8f,  1.0f, 0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f,  1.0f,
                };
                Ref<VertexBuffer> vb = VertexBuffer::Create(vertices, sizeof(vertices));
                BufferLayout layout  = {{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float4, "a_Color"}};
                vb->SetLayout(layout);
                m_TriangleVertexArray->AddVertexBuffer(vb);

                // Index buffer
                Ref<IndexBuffer> ib = IndexBuffer::Create({0, 1, 2});
                m_TriangleVertexArray->SetIndexBuffer(ib);
            }
            {
                // Square
                m_SquareVertexArray = VertexArray::Create();

                float vertices[5 * 4] = {-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,
                                         0.5f,  0.5f,  0.0f, 1.0f, 1.0f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f};
                Ref<VertexBuffer> vb  = VertexBuffer::Create(vertices, sizeof(vertices));
                vb->SetLayout({{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float2, "a_TexCoord"}});
                m_SquareVertexArray->AddVertexBuffer(vb);

                Ref<IndexBuffer> ib = IndexBuffer::Create({0, 1, 2, 2, 3, 0});
                m_SquareVertexArray->SetIndexBuffer(ib);
            }

            {
                // Cube
                m_CubeMesh = AssetManager::Load<Mesh>("models/cube.obj");
                m_CubeMaterial = CreateRef<Material>();
                m_CubeMaterial->albedo = AssetManager::Load<Texture>("textures/uv.png");
            }

            // Viewport
            FramebufferSpecification spec;
            spec.width  = Application::Get().GetWindow().GetWidth();
            spec.height = Application::Get().GetWindow().GetHeight();
            m_Viewport  = CreateRef<Viewport>(spec);

            // Ship
            m_ShipMesh = AssetManager::Load<Mesh>("models/colonial_fighter_red_fox/colonial_fighter_red_fox.obj");

            m_Environment = CreateRef<Environment>("textures/space_1k.hdr");
        }
        void LoadShaders() 
        { 
            //m_FlatColorShader = AssetManager::Load<Shader>("shaders/color.glsl", false);
            m_TextureShader   = AssetManager::Load<Shader>("shaders/texture.glsl", false);
            m_PBRShader       = AssetManager::Load<Shader>("shaders/pbr.glsl", false); 
        }

        inline static float timer = 0.0f;
        void OnUpdate() override
        {
            EVA_PROFILE_FUNCTION();

            auto dt = Platform::GetDeltaTime();
            m_FrameTimes.Add(dt);

            if (m_Viewport->IsFocused())
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
            if (m_Viewport->Update())
            {
                EVA_PROFILE_SCOPE("Resize viewport");
                m_OrtoCameraController.OnResize(m_Viewport->GetSize().x, m_Viewport->GetSize().y);
                m_PersCameraController.OnResize(m_Viewport->GetSize().x, m_Viewport->GetSize().y);
            }
            m_Viewport->Bind();
            RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
            RenderCommand::Clear();

            // Renderer::BeginScene(m_OrtoCameraController.GetCamera());
            Renderer::BeginScene(m_PersCameraController.GetCamera(), m_Environment, m_Lights);

            // Sky
            m_Environment->DrawSkyBox();

            // Squares
            /*m_FlatColorShader->Bind();
            m_FlatColorShader->SetUniformFloat3("u_Color", m_SquareColor);

            auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
            for (size_t y = 0; y < 20; y++)
            {
                for (size_t x = 0; x < 20; x++)
                {
                    auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(2 + x * 0.11f, y * 0.11f, 0.0f)) * scale;
                    Renderer::Submit(m_FlatColorShader, m_SquareVertexArray, transform);
                }
            }*/

            auto transform = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f));
            /*m_TextureShader->Bind();
            m_TextureShader->ResetTextureUnit();
            m_TextureShader->BindTexture("u_AlbedoMap", m_Texture);
            Renderer::Submit(m_TextureShader, m_SquareVertexArray, transform);*/

            // Cube
            m_TextureShader->Bind();
            m_TextureShader->ResetTextureUnit();
            Renderer::Submit(m_TextureShader, m_CubeMesh->GetVertexArray(), transform, m_CubeMaterial);

            // Ship
            m_PBRShader->Bind();
            m_PBRShader->ResetTextureUnit();
            Renderer::Submit(m_PBRShader, m_ShipMesh->GetVertexArray(), glm::identity<glm::mat4>(), m_ShipMesh->GetMaterial());

            Renderer::EndScene();
            m_Viewport->Unbind();
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
            if (ImGui::Button("Remove light") && !m_Lights.empty()) m_Lights.erase(m_Lights.end() - 1);
            for (auto& l : m_Lights)
            {
                l.Inspector();
                ImGui::Spacing();
                ImGui::Spacing();
            }
            ImGui::End();

            // Viewport
            m_Viewport->Draw();
        }
    };
} // namespace EVA
