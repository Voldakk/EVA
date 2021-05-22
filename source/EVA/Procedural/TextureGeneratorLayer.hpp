#pragma once

#include "EVA.hpp"
#include "NodeEditor.hpp"
#include "TextueNodes.hpp"
#include "EVA/Assets/FileDialog.hpp"

namespace EVA
{
    class TextureGeneratorLayer : public EVA::Layer
    {
      public:
        TextureGeneratorLayer();
        ~TextureGeneratorLayer() = default;

        void OnUpdate() override;
        void OnEvent(EVA::Event& e) override;
        void OnImGuiRender() override;

        void New();
        void Load(std::filesystem::path path);
        void Save(std::filesystem::path path);

      private:
        Viewport m_Viewport;
        Ref<Environment> m_Environment;
        OrbitalCameraController m_CameraController;
        Ref<Mesh> m_Mesh;
        Ref<Shader> m_Shader;
        Material m_Material;
        Ref<TextureNodes::Output> m_OutputNode;

        NE::NodeEditor m_NodeEditor;
    };

    TextureGeneratorLayer::TextureGeneratorLayer() :
      m_CameraController(Application::Get().GetWindow().GetAspect(), glm::vec3(0.0f), 30.0f, 0.0f, 5.0f)
    {
        m_NodeEditor.AddCompatiblePinType(NE::NodeEditor::GetPinType<Ref<Texture>, 1, 4>(), NE::NodeEditor::GetPinType<Ref<Texture>, 1>());
        m_NodeEditor.AddCompatiblePinType(NE::NodeEditor::GetPinType<Ref<Texture>, 1, 4>(), NE::NodeEditor::GetPinType<Ref<Texture>, 4>());

        m_NodeEditor.GetStyle().SetPinColor<Ref<Texture>, 1>({0.5f, 0.5f, 0.5f});
        m_NodeEditor.GetStyle().SetPinColor<Ref<Texture>, 4>({0.9f, 0.7f, 0.1f});
        m_NodeEditor.GetStyle().SetPinColor<Ref<Texture>, 1, 4>({0.9f, 0.8f, 0.5f});

        m_Environment = CreateRef<Environment>("textures/parking_lot_2_1k.hdr");
        m_Mesh        = AssetManager::Load<Mesh>("models/cube.obj");
        m_Shader      = AssetManager::Load<Shader>("shaders/pbr.glsl");

        New();
    }

    void TextureGeneratorLayer::OnUpdate()
    {
        if (m_Viewport.Update()) { m_CameraController.OnResize(m_Viewport.GetSize().x, m_Viewport.GetSize().y); }

        if (m_Viewport.IsFocused())
        {
            if (Input::IsMouseButtonPressed(MouseCode::Button0)) m_CameraController.OnUpdate();
        }

        m_Viewport.Bind();
        RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 1});
        RenderCommand::Clear();
        Renderer::BeginScene(m_CameraController.GetCamera(), m_Environment);

        m_Environment->DrawSkyBox();

        m_Material.albedo           = m_OutputNode->GetTexture(0);
        m_Material.normal           = m_OutputNode->GetTexture(1);
        m_Material.metallic         = m_OutputNode->GetTexture(2);
        m_Material.roughness        = m_OutputNode->GetTexture(3);
        m_Material.ambientOcclusion = m_OutputNode->GetTexture(4);
        m_Material.emissive         = m_OutputNode->GetTexture(5);
        m_Material.height           = m_OutputNode->GetTexture(6);

        static Transform t;
        t.Rotate(Platform::GetDeltaTime() * 10.0f);
        if (m_Shader)
        {
            m_Shader->Bind();
            m_Shader->ResetTextureUnit();
            m_Material.Bind(m_Shader);
            Renderer::Submit(m_Shader, m_Mesh->GetVertexArray(), t.GetModelMatrix());
        }

        Renderer::EndScene();
        m_Viewport.Unbind();
    }

    void TextureGeneratorLayer::OnEvent(EVA::Event& e)
    {
        EVA_PROFILE_FUNCTION();

        m_CameraController.OnEvent(e);
    }

    void TextureGeneratorLayer::OnImGuiRender()
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        ImGui::BeginMainMenuBar();
        if (ImGui::MenuItem("New")) { New(); }
        if (ImGui::MenuItem("Load"))
        {
            auto path = FileDialog::OpenFile({"Node graph", "*.graph"});
            if (!path.empty()) Load(path[0]);
        }
        if (ImGui::MenuItem("Save"))
        {
            auto path = FileDialog::SaveFile({".graph"});
            if (!path.empty()) Save(path);
        }
        ImGui::EndMainMenuBar();

        ImGui::Begin("Node editor");
        m_NodeEditor.Draw();
        ImGui::End();

        ImGui::Begin("Nodes");
        if (ImGui::Button("Input")) { m_NodeEditor.AddNode<TextureNodes::Input>(); }
        if (ImGui::Button("Output")) { m_NodeEditor.AddNode<TextureNodes::Output>(); }
        if (ImGui::Button("Material output"))
        {
            std::vector<std::string> names = {"Albedo", "Normal", "Metallic", "Roughness", "AO", "Emissive", "Height"};
            m_OutputNode                   = CreateRef<TextureNodes::Output>(names);
            m_NodeEditor.AddNode(m_OutputNode, {200.0f, 0.0f});
        }
        if (ImGui::Button("Passthrough")) { m_NodeEditor.AddNode<TextureNodes::Passthrough>(); }
        ImGui::Spacing();
        if (ImGui::Button("Uniform")) { m_NodeEditor.AddNode<TextureNodes::Uniform>(); }
        if (ImGui::Button("Bricks")) { m_NodeEditor.AddNode<TextureNodes::Bricks>(); }
        ImGui::Spacing();
        if (ImGui::Button("Gradient noise")) { m_NodeEditor.AddNode<TextureNodes::GradientNoise>(); }
        if (ImGui::Button("Voronoi noise")) { m_NodeEditor.AddNode<TextureNodes::VoronoiNoise>(); }
        if (ImGui::Button("Worley noise")) { m_NodeEditor.AddNode<TextureNodes::WorleyNoise>(); }
        if (ImGui::Button("Worley noise 2")) { m_NodeEditor.AddNode<TextureNodes::WorleyNoise2>(); }
        ImGui::Spacing();
        if (ImGui::Button("Blend")) { m_NodeEditor.AddNode<TextureNodes::Blend>(); }
        if (ImGui::Button("Levels")) { m_NodeEditor.AddNode<TextureNodes::Levels>(); }
        if (ImGui::Button("Gradient map")) { m_NodeEditor.AddNode<TextureNodes::GradientMap>(); }
        if (ImGui::Button("Directional warp")) { m_NodeEditor.AddNode<TextureNodes::DirectionalWarp>(); }
        if (ImGui::Button("Non uniform directional warp")) { m_NodeEditor.AddNode<TextureNodes::NonUniformDirectionalWarp>(); }
        ImGui::Spacing();
        if (ImGui::Button("Height to normal")) { m_NodeEditor.AddNode<TextureNodes::HeightToNormal>(); }
        ImGui::End();

        ImGui::Begin("Selected");
        auto& selected = m_NodeEditor.GetSelectedNodes();
        if (!selected.empty())
        {
            DataObject d(DataMode::Inspector);
            reinterpret_cast<TextureNodes::TextureNode*>(selected[0])->Serialize(d);
        }
        ImGui::End();

        ImGui::Begin("Texture");
        if (!selected.empty())
        {
            auto texture       = reinterpret_cast<TextureNodes::TextureNode*>(selected[0])->GetTexture();
            uint32_t textureId = 0;
            if (texture != nullptr) textureId = texture->GetRendererId();
            auto size = ImGui::GetContentRegionAvail();
            float max = glm::min(size.x, size.y);
            ImGui::Image(*reinterpret_cast<void**>(&textureId), ImVec2 {max, max}, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});
        }
        ImGui::End();

        ImGui::Begin("Settings");
        if (ImGui::Button("Reload shader")) 
        { m_Shader = AssetManager::Load<Shader>(m_Shader->GetPath(), false); 
        }
        InspectorFields::Default("Shader", m_Shader);

        ImGui::SliderFloat("Height scale", &m_Material.heightScale, 0.0f, 0.1f);
        ImGui::End();

        m_Viewport.Draw();
    }

    void TextureGeneratorLayer::New()
    {
        m_NodeEditor.New();
        std::vector<std::string> names = {"Albedo", "Normal", "Metallic", "Roughness", "AO", "Emissive", "Height"};
        m_OutputNode                   = CreateRef<TextureNodes::Output>(names);
        m_NodeEditor.AddNode(m_OutputNode, {200.0f, 0.0f});
    }

    void TextureGeneratorLayer::Load(std::filesystem::path path)
    {
        m_NodeEditor.Load(path);
        auto outputs = m_NodeEditor.GetNodesOfType<TextureNodes::Output>();
        if (!outputs.empty()) { m_OutputNode = outputs[0]; }
    }

    void TextureGeneratorLayer::Save(std::filesystem::path path) { m_NodeEditor.Save(path); }

} // namespace EVA