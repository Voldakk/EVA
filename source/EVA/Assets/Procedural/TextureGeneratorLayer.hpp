#pragma once

#include "EVA.hpp"
#include "NodeEditor.hpp"
#include "TextueNode.hpp"

namespace EVA
{
    inline static const uint32_t PinFloat = NE::NodeEditor::GetPinType<float>();

    template<class T>
    class ValueNode : public NE::Node
    {
      public:
        ValueNode(const T& value) : m_Value(value) {}
        virtual ~ValueNode() = default;

        void SetupNode() override
        {
            name = "Value";
            AddPins({{NE::PinKind::Output, NE::NodeEditor::GetPinType<T>(), std::to_string(m_Value)}});
        }

        void Process() override { SetOutputData(0, &m_Value); }

      private:
        T m_Value {};
    };

    template<class T>
    class DisplayNode : public NE::Node
    {
      public:
        DisplayNode()          = default;
        virtual ~DisplayNode() = default;

        void SetupNode() override
        {
            name = "Value";
            AddPins({{NE::PinKind::Input, NE::NodeEditor::GetPinType<T>(), std::to_string(m_Value), true}});
        }

        void Process() override
        {
            m_Value        = GetInputData<T>(0);
            inputs[0].name = std::to_string(m_Value);
        }

      private:
        T m_Value {};
    };

    template<class T>
    class MaxNode : public NE::Node
    {
      public:
        MaxNode()          = default;
        virtual ~MaxNode() = default;

        void SetupNode() override
        {
            name = "Max";
            AddPins({{NE::PinKind::Input, PinFloat, "A", false}, {NE::PinKind::Input, PinFloat, "B", false}, {NE::PinKind::Output, PinFloat, "Max"}});
            SetOutputData(0, &m_Value);
        }

        void Process() override
        {
            const T& a = GetInputData(0, T {});
            const T& b = GetInputData(1, T {});

            m_Value = glm::max(a, b);
        }

      private:
        T m_Value {};
    };
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

    TextureGeneratorLayer::TextureGeneratorLayer() {}

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
        if (ImGui::Button("Uniform color")) { m_NodeEditor.AddNode<TextureNodes::UniformColor>(); }
        if (ImGui::Button("Uniform grayscale")) { m_NodeEditor.AddNode<TextureNodes::UniformGrayscale>(); }
        ImGui::Spacing();
        if (ImGui::Button("Voronoi noise")) { m_NodeEditor.AddNode<TextureNodes::VoronoiNoise>(); }
        if (ImGui::Button("Gradient noise")) { m_NodeEditor.AddNode<TextureNodes::GradientNoise>(); }
        if (ImGui::Button("Bricks")) { m_NodeEditor.AddNode<TextureNodes::Bricks>(); }
        ImGui::Spacing();
        if (ImGui::Button("Blend")) { m_NodeEditor.AddNode<TextureNodes::BlendGrayscale>(); }
        if (ImGui::Button("Levels")) { m_NodeEditor.AddNode<TextureNodes::LevelsGrayscale>(); }
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