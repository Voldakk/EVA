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
        virtual ~TextureGeneratorLayer() { EVA_PROFILE_FUNCTION(); };

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
        m_Mesh        = AssetManager::Load<Mesh>("models/cube_bevel.obj");
        m_Shader      = AssetManager::Load<Shader>("shaders/pbr.glsl");

        m_Material.heightScale = 0.015f;

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
        // t.Rotate(Platform::GetDeltaTime() * 10.0f);
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

        ImGui::Text("CCL");
        if (ImGui::Button("CCL")) { m_NodeEditor.AddNode<TextureNodes::CCL>(); }
        if (ImGui::Button("CCL to gradient")) { m_NodeEditor.AddNode<TextureNodes::CCLToGradient>(); }
        if (ImGui::Button("CCL to index")) { m_NodeEditor.AddNode<TextureNodes::CCLToIndex>(); }
        if (ImGui::Button("CCL to map")) { m_NodeEditor.AddNode<TextureNodes::CCLMap>(); }
        if (ImGui::Button("CCL to position")) { m_NodeEditor.AddNode<TextureNodes::CCLToPosition>(); }
        if (ImGui::Button("CCL to random")) { m_NodeEditor.AddNode<TextureNodes::CCLToRandom>(); }
        ImGui::Spacing();

        ImGui::Text("Convert");
        if (ImGui::Button("Distance")) { m_NodeEditor.AddNode<TextureNodes::Distance>(); }
        if (ImGui::Button("Edge detect")) { m_NodeEditor.AddNode<TextureNodes::EdgeDetect>(); }
        if (ImGui::Button("Height to AO")) { m_NodeEditor.AddNode<TextureNodes::HeightToAmbientOcclusion>(); }
        if (ImGui::Button("Height to curvature")) { m_NodeEditor.AddNode<TextureNodes::HeightToCurvature>(); }
        if (ImGui::Button("Height to normal")) { m_NodeEditor.AddNode<TextureNodes::HeightToNormal>(); }
        ImGui::Spacing();

        ImGui::Text("Filters");
        if (ImGui::Button("Gaussian blur 5x5")) { m_NodeEditor.AddNode<TextureNodes::GaussianBlur5>(); }
        ImGui::Spacing();

        ImGui::Text("Generators");
        if (ImGui::Button("Bricks")) { m_NodeEditor.AddNode<TextureNodes::Bricks>(); }
        if (ImGui::Button("Shape")) { m_NodeEditor.AddNode<TextureNodes::Shape>(); }
        if (ImGui::Button("Tile generator")) { m_NodeEditor.AddNode<TextureNodes::TileGenerator>(); }
        if (ImGui::Button("Uniform")) { m_NodeEditor.AddNode<TextureNodes::Uniform>(); }
        ImGui::Spacing();

        ImGui::Text("Modifiers");
        if (ImGui::Button("Blend")) { m_NodeEditor.AddNode<TextureNodes::Blend>(); }
        if (ImGui::Button("Blend normal")) { m_NodeEditor.AddNode<TextureNodes::BlendNormal>(); }
        if (ImGui::Button("Curve")) { m_NodeEditor.AddNode<TextureNodes::Curve>(); }
        if (ImGui::Button("Directional warp")) { m_NodeEditor.AddNode<TextureNodes::DirectionalWarp>(); }
        if (ImGui::Button("Non-uniform directional warp")) { m_NodeEditor.AddNode<TextureNodes::DirectionalWarpNonUniform>(); }
        if (ImGui::Button("Gradient map")) { m_NodeEditor.AddNode<TextureNodes::GradientMap>(); }
        if (ImGui::Button("Levels")) { m_NodeEditor.AddNode<TextureNodes::Levels>(); }
        if (ImGui::Button("Quad transform")) { m_NodeEditor.AddNode<TextureNodes::QuadTransform>(); } 
        ImGui::Spacing();

        ImGui::Text("Noise");
        if (ImGui::Button("Gradient noise")) { m_NodeEditor.AddNode<TextureNodes::GradientNoise>(); }
        if (ImGui::Button("Voronoi noise")) { m_NodeEditor.AddNode<TextureNodes::VoronoiNoise>(); }
        if (ImGui::Button("Worley noise")) { m_NodeEditor.AddNode<TextureNodes::WorleyNoise>(); }
        ImGui::Spacing();

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
            auto tNode         = reinterpret_cast<TextureNodes::TextureNode*>(selected[0]);
            auto texture       = tNode ? tNode->GetTexture() : nullptr;
            uint32_t textureId = 0;
            if (texture != nullptr) textureId = texture->GetRendererId();
            auto size = ImGui::GetContentRegionAvail();
            float max = glm::min(size.x, size.y);
            ImGui::Image(*reinterpret_cast<void**>(&textureId), ImVec2 {max, max}, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});
        }
        ImGui::End();

        ImGui::Begin("Settings");
        if (ImGui::Button("Navigate to content")) { m_NodeEditor.NavigateToContent(); }

        if (ImGui::Button("Reload shader")) { m_Shader = AssetManager::Load<Shader>(m_Shader->GetPath(), false); }
        InspectorFields::Default("Shader", m_Shader);

        if (ImGui::Button("Reload mesh")) { m_Mesh = AssetManager::Load<Mesh>(m_Mesh->GetPath(), false); }
        InspectorFields::Default("Mesh", m_Mesh);

        ImGui::Checkbox("Enable paralax", &m_Material.enableParalax);
        ImGui::Checkbox("Paralax cliping", &m_Material.paralaxClip);
        ImGui::SliderFloat("Height scale", &m_Material.heightScale, 0.0f, 0.1f);
        ImGui::SliderFloat2("Tiling", glm::value_ptr(m_Material.tiling), 0.0f, 10.0f);

        DataObject d(DataMode::Inspector);
        ImGui::Spacing();
        m_Environment->Serialize(d);

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