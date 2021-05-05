#pragma once

#include "EVA.hpp"
#include "NodeEditor.hpp"
#include "TextueNode.hpp"

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

      private:
        Viewport m_Viewport;
        Ref<Environment> m_Environment;
        OrbitalCameraController m_CameraController;
        Ref<Mesh> m_CubeMesh;
        Ref<Shader> m_PBRShader;
        Material m_Material;

        std::vector<Ref<TextureNodes::Output>> m_OutputNodes;
        Ref<TextureNodes::Output> m_Emissive;
        Ref<TextureNodes::Output> m_Normal;
        Ref<TextureNodes::Output> m_Metallic;
        Ref<TextureNodes::Output> m_Roughness;
        Ref<TextureNodes::Output> m_AO;

        NE::NodeEditor m_NodeEditor;
    };

    TextureGeneratorLayer::TextureGeneratorLayer() : m_CameraController(Application::Get().GetWindow().GetAspect(), glm::vec3(0.0f), 30.0f, 0.0f)
    { 
        m_NodeEditor.AddCompatiblePinType(NE::NodeEditor::GetPinType<Ref<Texture>, 1, 4>(), NE::NodeEditor::GetPinType<Ref<Texture>, 1>());
        m_NodeEditor.AddCompatiblePinType(NE::NodeEditor::GetPinType<Ref<Texture>, 1, 4>(), NE::NodeEditor::GetPinType<Ref<Texture>, 4>());

        m_NodeEditor.GetStyle().SetPinColor<Ref<Texture>, 1>({0.5f, 0.5f, 0.5f});
        m_NodeEditor.GetStyle().SetPinColor<Ref<Texture>, 4>({0.9f, 0.7f, 0.1f});
        m_NodeEditor.GetStyle().SetPinColor<Ref<Texture>, 1, 4>({0.9f, 0.8f, 0.5f});

        m_Environment = CreateRef<Environment>("./assets/textures/parking_lot_2_1k.hdr");
        m_CubeMesh    = Mesh::LoadMesh("./assets/models/cube.obj")[0];
        m_PBRShader   = Shader::Create("./assets/shaders/pbr.glsl");

        std::vector<std::string> names = {"Albedo", "Normal", "Metallic", "Roughness", "AO", "Emissive"};
        float pos = 0;
        for (const auto& name : names)
        {
            auto node       = CreateRef<TextureNodes::Output>(name);
            node->deletable = false;
            m_NodeEditor.AddNode(node, {200.0f, pos});
            m_OutputNodes.push_back(node);
            pos += 250.0f;
        }
    }

    void TextureGeneratorLayer::OnUpdate()
    {
        if (m_Viewport.Update()) 
        { 
            m_CameraController.OnResize(m_Viewport.GetSize().x, m_Viewport.GetSize().y); 
        }

        if (m_Viewport.IsFocused()) 
        { 
            if (Input::IsMouseButtonPressed(MouseCode::Button0))
                m_CameraController.OnUpdate();
        }

        m_Viewport.Bind();
        RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 1});
        RenderCommand::Clear();
        Renderer::BeginScene(m_CameraController.GetCamera(), m_Environment);

        m_Environment->DrawSkyBox();

        m_Material.albedo = m_OutputNodes[0]->GetTexture();
        m_Material.normal = m_OutputNodes[1]->GetTexture();
        m_Material.metallic = m_OutputNodes[2]->GetTexture();
        m_Material.roughness = m_OutputNodes[3]->GetTexture();
        m_Material.ambientOcclusion = m_OutputNodes[4]->GetTexture();
        m_Material.emissive = m_OutputNodes[5]->GetTexture();

        m_PBRShader->Bind();
        m_PBRShader->ResetTextureUnit();
        m_Material.Bind(m_PBRShader);
        Renderer::Submit(m_PBRShader, m_CubeMesh->GetVertexArray());

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

        ImGui::Begin("Node editor");
        m_NodeEditor.Draw();
        ImGui::End();

        ImGui::Begin("Nodes");
        if (ImGui::Button("Input")) { m_NodeEditor.AddNode<TextureNodes::Input>(); }
        if (ImGui::Button("Output")) { m_NodeEditor.AddNode<TextureNodes::Output>(); }
        if (ImGui::Button("Passthrough")) { m_NodeEditor.AddNode<TextureNodes::Passthrough>(); }
        ImGui::Spacing();
        if (ImGui::Button("Uniform")) { m_NodeEditor.AddNode<TextureNodes::Uniform>(); }
        if (ImGui::Button("Voronoi noise")) { m_NodeEditor.AddNode<TextureNodes::VoronoiNoise>(); }
        if (ImGui::Button("Gradient noise")) { m_NodeEditor.AddNode<TextureNodes::GradientNoise>(); }
        if (ImGui::Button("Bricks")) { m_NodeEditor.AddNode<TextureNodes::Bricks>(); }
        ImGui::Spacing();
        if (ImGui::Button("Blend")) { m_NodeEditor.AddNode<TextureNodes::Blend>(); }
        if (ImGui::Button("Levels")) { m_NodeEditor.AddNode<TextureNodes::Levels>(); }
        if (ImGui::Button("Gradient map")) { m_NodeEditor.AddNode<TextureNodes::GradientMap>(); }
        ImGui::Spacing();
        if (ImGui::Button("Height to normal")) { m_NodeEditor.AddNode<TextureNodes::HeightToNormal>(); }
        ImGui::End();

        ImGui::Begin("Selected");
        auto& selected = m_NodeEditor.GetSelectedNodes();
        if (!selected.empty()) { 
            DataObject d;
            d.mode = DataObject::DataMode::Inspector;
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

        m_Viewport.Draw();
    }

} // namespace EVA