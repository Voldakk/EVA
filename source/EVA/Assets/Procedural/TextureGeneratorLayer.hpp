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

        ImGui::Begin("Left pane");

        if (ImGui::Button("New float display")) { m_NodeEditor.AddNode<DisplayNode<float>>(); }


        static float f = 0.0f;
        ImGui::InputFloat("Value##f: ", &f);
        if (ImGui::Button("New float node")) { m_NodeEditor.AddNode<ValueNode<float>>(f); }

        static int i = 0;
        ImGui::InputInt("Value##i: ", &i);
        if (ImGui::Button("New int node")) { m_NodeEditor.AddNode<ValueNode<int>>(i); }

        if (ImGui::Button("New max float node")) { m_NodeEditor.AddNode<MaxNode<float>>(); }

        ImGui::Text("Texture nodes");
        if (ImGui::Button("New Passthrough")) { m_NodeEditor.AddNode<TextureNodes::PassthroughNode>(); }
        if (ImGui::Button("New Voronoi noise")) { m_NodeEditor.AddNode<TextureNodes::VoronoiNoiseNode>(); }
        if (ImGui::Button("New Gradient noise")) { m_NodeEditor.AddNode<TextureNodes::GradientNoiseNode>(); }
        if (ImGui::Button("New Blend")) { m_NodeEditor.AddNode<TextureNodes::BlendNode>(); }
        if (ImGui::Button("New Brick")) { m_NodeEditor.AddNode<TextureNodes::BrickNode>(); }

        ImGui::End();
    }

} // namespace EVA