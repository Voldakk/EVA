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
        void OnImGuiRender() override;

      private:
        NE::NodeEditor m_NodeEditor;
    };

    TextureGeneratorLayer::TextureGeneratorLayer() 
    { 
        m_NodeEditor.AddCompatiblePinType(NE::NodeEditor::GetPinType<Ref<Texture>, 1, 4>(), NE::NodeEditor::GetPinType<Ref<Texture>, 1>());
        m_NodeEditor.AddCompatiblePinType(NE::NodeEditor::GetPinType<Ref<Texture>, 1, 4>(), NE::NodeEditor::GetPinType<Ref<Texture>, 4>());

        m_NodeEditor.GetStyle().SetPinColor<Ref<Texture>, 1>({0.5f, 0.5f, 0.5f});
        m_NodeEditor.GetStyle().SetPinColor<Ref<Texture>, 4>({0.9f, 0.7f, 0.1f});
        m_NodeEditor.GetStyle().SetPinColor<Ref<Texture>, 1, 4>({0.9f, 0.8f, 0.5f});
    }

    void TextureGeneratorLayer::OnUpdate()
    {
        RenderCommand::SetClearColor({0.0f, 0.0f, 0.0f, 1});
        RenderCommand::Clear();
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
    }

} // namespace EVA