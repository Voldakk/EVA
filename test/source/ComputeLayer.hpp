#pragma once

#include "EVA.hpp"
#include "ShipController.hpp"
#include "Platform/OpenGL/OpenGLContext.hpp"
#include "EVA/Editor/Viewport.hpp"
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
    static std::uniform_real_distribution<float> dist(0.1f, 0.5f);
    return dist(e2);
}

namespace EVA
{
    template<KeyCode keyCode>
    bool IsKeyPressed()
    {
        static bool canClick = true;
        if (Input::IsKeyPressed(keyCode))
        {
            if (canClick)
            {
                canClick = false;
                return true;
            }

            canClick = false;
        }
        else
        {
            canClick = true;
        }
        return false;
    }

    struct SceneParams
    {
        float surfaceDist = 0.001f;
        float maxDist     = 100.0f;
        int maxSteps      = 100;
        float normalEps   = 0.001f;

        float aoIntensity = 4.0f;
        float aoStepSize  = 0.1f;
        int aoSteps       = 10;

        bool mandelbulbEnable    = true;
        int mandelbulbIterations = 10;
        float mandelbulbPower    = 8.0f;
        float mandelbulbScale    = 3.0f;

        bool tetrahedronEnable = true;
        int tetrahedronIterations = 20;
        float tetrahedronScale    = 2.0f;
        float tetrahedronOffset   = 2.0f;

        glm::vec3 albedo = glm::vec3(1.0f);
        float metallic  = 0.0f;
        float roughness = 0.5f;

        void Inspector()
        {
            ImGui::Text("Scene");

            ImGui::SliderFloat("Surface dist", &surfaceDist, 0, 1);
            ImGui::SliderFloat("Max dist", &maxDist, 1, 1000);
            ImGui::SliderInt("Max steps", &maxSteps, 1, 1000);
            ImGui::SliderFloat("Normal eps", &normalEps, 0, 1);

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::Text("AO");
            ImGui::SliderFloat("Intensity##AO", &aoIntensity, 0, 10);
            ImGui::SliderFloat("Step size##AO", &aoStepSize, 0, 10);
            ImGui::SliderInt("Steps##AO", &aoSteps, 0, 10);

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::Text("Mandelbulb");
            ImGui::Checkbox("Enable##Mandelbulb", &mandelbulbEnable);
            ImGui::SliderInt("Iterations##Mandelbulb", &mandelbulbIterations, 0, 20);
            ImGui::SliderFloat("Power##Mandelbulb", &mandelbulbPower, 0, 100);
            ImGui::SliderFloat("Scale##Mandelbulb", &mandelbulbScale, 0, 100);

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::Text("Tetrahedron");
            ImGui::Checkbox("Enable##Tetrahedron", &tetrahedronEnable);
            ImGui::SliderInt("Iterations##Tetrahedron", &tetrahedronIterations, 0, 50);
            ImGui::SliderFloat("Scale##Tetrahedron", &tetrahedronScale, 0, 100);
            ImGui::SliderFloat("Offset##Tetrahedron", &tetrahedronOffset, 0, 10);

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::Text("Material");
            ImGui::ColorEdit3("Albedo", glm::value_ptr(albedo));
            ImGui::SliderFloat("Metallic", &metallic, 0, 10);
            ImGui::SliderFloat("Roughness", &roughness, 0, 1);
        }

        void SetUniforms(Ref<Shader>& shader) 
        {
            shader->SetUniformFloat("u_SurfaceDist", surfaceDist);
            shader->SetUniformFloat("u_MaxDist", maxDist);
            shader->SetUniformInt("u_MaxSteps", maxSteps);

            shader->SetUniformFloat("u_NormalEps", normalEps);

            shader->SetUniformFloat("u_AoIntensity", aoIntensity);
            shader->SetUniformFloat("u_AoStepSize", aoStepSize);
            shader->SetUniformInt("u_AoSteps", aoSteps);

            shader->SetUniformBool("u_MandelbulbEnable", mandelbulbEnable);
            shader->SetUniformInt("u_MandelbulbIterations", mandelbulbIterations);
            shader->SetUniformFloat("u_MandelbulbPower", mandelbulbPower);
            shader->SetUniformFloat("u_MandelbulbScale", mandelbulbScale);

            shader->SetUniformBool("u_TetrahedronEnable", tetrahedronEnable);
            shader->SetUniformInt("u_TetrahedronIterations", tetrahedronIterations);
            shader->SetUniformFloat("u_TetrahedronScale", tetrahedronScale);
            shader->SetUniformFloat("u_TetrahedronOffset", tetrahedronOffset);

            shader->SetUniformFloat3("u_Albedo", albedo);
            shader->SetUniformFloat("u_Metallic", metallic);
            shader->SetUniformFloat("u_Roughness", roughness);
        }
    };

    class ComputeLayer : public EVA::Layer
    {
        SlidingWindow<float> m_FrameTimes;

        Ref<Texture> m_ComputeTexture;
        Ref<Shader> m_ComputeShader;

        Ref<ShaderStorageBuffer> m_Ssbo;
        std::vector<glm::vec4> m_SsboData;
        size_t m_MaxObjects = 100;

        Ref<Shader> m_PBRShader;
        Ref<Mesh> m_ShipMesh;
        Ref<ShipController> m_ShipController;
        bool m_InShip = false;

        Ref<Environment> m_Environment;

        Ref<PerspectiveCameraController> m_CameraController;
        Ref<ChaseCameraController> m_ChaseCameraController;

        SceneParams m_SceneParams;
        std::vector<Light> m_Lights;

        Ref<Viewport> m_Viewport;

    public:
        ComputeLayer() :
            Layer("Compute"),
            m_FrameTimes(10)
        {
            EVA_PROFILE_FUNCTION();

            LoadShaders();

            // Viewport
            FramebufferSpecification spec;
            spec.width = Application::Get().GetWindow().GetWidth();
            spec.height = Application::Get().GetWindow().GetHeight();
            m_Viewport = CreateRef<Viewport>(spec);

            // Compute
            m_ComputeTexture = TextureManager::CreateTexture(512, 512, TextureFormat::RGBA8);
            m_Ssbo = ShaderStorageBuffer::Create(m_SsboData.data(), m_SsboData.size() * sizeof(glm::vec4));

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_Ssbo->GetRendererId());

            // Ship
            m_ShipMesh       = AssetManager::Load<Mesh>("models/colonial_fighter_red_fox/colonial_fighter_red_fox.obj");
            m_ShipController = CreateRef<ShipController>(glm::vec3(5.0f, 0.0f, 0.0f));

            // Enviroment
            m_Environment = CreateRef<Environment>("textures/space_1k.hdr");

            // Cameras
            m_CameraController = CreateRef<PerspectiveCameraController>(glm::vec3(0, 2, -5), 0.0f, 0.0f, Application::Get().GetWindow().GetAspect());
            m_ChaseCameraController = CreateRef<ChaseCameraController>(m_CameraController->GetCamera(), m_CameraController->GetTransform(), m_ShipController->GetTransform());
            
            // Lights
            Light l;
            l.type = Light::Type::Point;
            l.position = glm::vec3(0.0f, 1.0f, 0.0f);
            m_Lights.push_back(l);
        }

        void LoadShaders()
        {
            m_ComputeShader = AssetManager::Load<Shader>("shaders/compute.glsl");
            m_PBRShader     = AssetManager::Load<Shader>("shaders/pbr.glsl");
        }

        inline static float timer = 0.0f;
        void OnUpdate() override
        {
            EVA_PROFILE_FUNCTION();

            auto dt = EVA::Platform::GetDeltaTime();
            m_FrameTimes.Add(dt);

            if (IsKeyPressed<KeyCode::Escape>())
            {
                if (Input::GetCursorMode() == Input::CursorMode::Disabled)
                    Input::SetCursorMode(Input::CursorMode::Normal);
                else
                    Input::SetCursorMode(Input::CursorMode::Disabled);
            }  

            // Update
            if (m_Viewport->IsFocused())
            {
                EVA_PROFILE_SCOPE("Viewport update");

                if (Input::GetCursorMode() == Input::CursorMode::Disabled) 
                {
                    if (m_InShip) 
                    { 
                        m_ShipController->OnUpdate();
                        m_ChaseCameraController->OnUpdate();
                    }
                    else
                    {
                        m_CameraController->OnUpdate();
                    }
                }
            }
            
            {
                EVA_PROFILE_SCOPE("SSBO data");
                if (m_SsboData.size() < m_MaxObjects) { 
                    m_SsboData.push_back(glm::vec4(randomFloat(), randomFloat(), randomFloat(), randomRadius()));
                    m_Ssbo->BufferData(m_SsboData.data(), m_SsboData.size() * sizeof(glm::vec4));
                }
                else
                {
                    m_SsboData.erase(m_SsboData.begin(), m_SsboData.begin() + 5);
                    m_Ssbo->BufferData(m_SsboData.data(), m_SsboData.size() * sizeof(glm::vec4));
                }
            }

            // Render
            m_Viewport->Update();
            if (m_Viewport->GetSize() != m_ComputeTexture->GetSize())
            {
                EVA_PROFILE_SCOPE("Resize viewport");
                m_ComputeTexture = EVA::TextureManager::CreateTexture(m_Viewport->GetSize().x, m_Viewport->GetSize().y, TextureFormat::RGBA8);
                m_Viewport->SetTexture(m_ComputeTexture->GetRendererId());
            }

            {
                EVA_PROFILE_SCOPE("Render");

                m_Viewport->Bind();
                RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 1});
                RenderCommand::Clear();

                Renderer::BeginScene(m_CameraController->GetCamera(), m_Environment, m_Lights);

                m_PBRShader->Bind();
                m_PBRShader->ResetTextureUnit();
                m_ShipMesh->GetMaterial()->Bind(m_PBRShader);

                Renderer::Submit(m_PBRShader, m_ShipMesh->GetVertexArray(), m_ShipController->GetTransform().GetModelMatrix());

                Renderer::EndScene();
                m_Viewport->Unbind();
            }

            {
                EVA_PROFILE_SCOPE("Render ray");

                size_t numPixels   = m_ComputeTexture->GetWidth() * m_ComputeTexture->GetHeight();
                size_t numObjects  = glm::min(m_SsboData.size(), m_MaxObjects);

                auto maxWorkGroupSize = OpenGLContext::MaxComputeWorkGroupSize();
                size_t workGroupSize  = glm::min(numPixels, (size_t)maxWorkGroupSize.x);
                size_t numWorkGroups  = (size_t)glm::ceil(numPixels / (float)workGroupSize);

                m_ComputeShader->Bind();
                m_ComputeShader->ResetTextureUnit();
                m_ComputeShader->SetUniformFloat("u_Time", Platform::GetTime());

                m_ComputeShader->SetUniformMat4("u_ViewProjection", m_CameraController->GetCamera().GetViewProjectionMatrix());
                m_ComputeShader->SetUniformFloat("u_CameraNear", m_CameraController->GetNearPlane());
                m_ComputeShader->SetUniformFloat("u_CameraFar", m_CameraController->GetFarPlane());

                m_SceneParams.SetUniforms(m_ComputeShader);

                m_ComputeShader->BindTexture("u_EnvMap", m_Environment->GetEquirectangularMap());
                m_ComputeShader->BindTexture("u_IrradianceMap", m_Environment->GetIrradianceMap());
                m_ComputeShader->BindTexture("u_PrefilterMap", m_Environment->GetPreFilterMap());
                m_ComputeShader->BindTexture("u_BrdfLUT", m_Environment->GetBrdfLUT());


                m_ComputeShader->SetUniformInt("u_NumLights", m_Lights.size());
                for (size_t i = 0; i < m_Lights.size(); i++) 
                {
                    const auto uniformName = "u_AllLights[" + std::to_string(i) + "].";
                    m_Lights[i].SetUniforms(m_ComputeShader, uniformName);
                }

                m_ComputeShader->BindTexture("u_FbColor", TextureTarget::Texture2D, m_Viewport->GetFramebuffer()->GetColorAttachmentRendererId());
                m_ComputeShader->BindTexture("u_FbDepth", TextureTarget::Texture2D, m_Viewport->GetFramebuffer()->GetDepthAttachmentRendererId());

                m_ComputeShader->SetUniformInt("objectBufferCount", numObjects);

                m_ComputeShader->BindImageTexture(0, m_ComputeTexture, Access::WriteOnly);
                m_ComputeShader->DispatchCompute(numWorkGroups, 1, 1, workGroupSize, 1, 1);
            }
        }

        void OnEvent(EVA::Event& e) override 
        { 
            EVA_PROFILE_FUNCTION();
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(ComputeLayer::OnKeyPressed));

            m_CameraController->OnEvent(e);

            if (m_InShip) 
            {
                m_ChaseCameraController->OnEvent(e);
            }
        }

        bool OnKeyPressed(KeyPressedEvent& e)
        {
            if(e.GetKeyCode() == KeyCode::Enter) 
            { 
                m_InShip = !m_InShip;
            }
            return false;
        }

        void OnImGuiRender() override
        {
            EVA_PROFILE_FUNCTION();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            auto avgFrameTime = m_FrameTimes.GetAverage();
            ImGui::Begin("Metrics");
            ImGui::Text("FPS: %.2f", 1.0f / avgFrameTime);
            ImGui::Text("Frame time: %.2f ms", avgFrameTime * 1000);
            ImGui::Text("Objects: %lu / %lu", m_SsboData.size(), m_MaxObjects);
            ImGui::End();

            ImGui::Begin("Settings");
            if (ImGui::Button("Reload shaders")) LoadShaders();
            ImGui::Spacing();
            ImGui::Spacing();
            m_SceneParams.Inspector();
            ImGui::Spacing();
            ImGui::Spacing();
            m_CameraController->Inspector();
            ImGui::Spacing();
            ImGui::Spacing();
            m_ShipController->Inspector();
            ImGui::Spacing();
            ImGui::Spacing();
            m_ChaseCameraController->Inspector();
            ImGui::Spacing();
            ImGui::Spacing();
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
            m_Viewport->Draw();
        }
    };
} // namespace EVA
