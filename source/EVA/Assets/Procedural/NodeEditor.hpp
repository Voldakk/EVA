#pragma once

#include <imgui_node_editor.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include "EVA/Assets/ISerializeable.hpp"

namespace EVA::NE
{
    namespace NE  = ax::NodeEditor;
    using PinKind = NE::PinKind;

    enum class InputState
    {
        Unchanged,
        Changed,
        Pending
    };

    class NodeEditor;
    class Node;

    struct InputPinInfo
    {
        std::string name;
        bool required = true;
    };

    struct OutputPinInfo
    {
        std::string name;
        void* data = nullptr;
    };

    bool SerializeId(DataObject& data, const std::string& key, NE::PinId id)
    {
        auto value   = id.Get();
        bool changed = data.Serialize(key, value);
        id           = NE::PinId(value);
        return changed;
    }
    bool SerializeId(DataObject& data, const std::string& key, NE::NodeId id)
    {
        auto value   = id.Get();
        bool changed = data.Serialize(key, value);
        id           = NE::NodeId(value);
        return changed;
    }
    bool SerializeId(DataObject& data, const std::string& key, NE::LinkId id)
    {
        auto value   = id.Get();
        bool changed = data.Serialize(key, value);
        id           = NE::LinkId(value);
        return changed;
    }
    bool SerializeKind(DataObject& data, const std::string& key, NE::PinKind kind)
    {
        int value   = static_cast<int>(kind);
        bool changed = data.Serialize(key, value);
        kind         = static_cast<NE::PinKind>(value);
        return changed;
    }

    struct Pin : public ISerializeable
    {
        NE::PinId id;
        NE::PinKind kind;
        uint32_t type;
        std::string name;
        Node* node;

        std::vector<Pin*> connectedPins;

        // Input
        bool required         = true;
        InputState inputState = InputState::Changed;

        // Output
        void* outputData = nullptr;

        Pin() = default;
        Pin(NE::PinId id, NE::PinKind kind, uint32_t type, Node* node, const std::string& name, bool required = true) :
          id(id), kind(kind), type(type), node(node), name(name), required(required)
        {
        }

        void Serialize(DataObject& data) override
        {
            SerializeId(data, "id", id);
            SerializeKind(data, "kind", kind);
            data.Serialize("type", type);
            data.Serialize("name", name);
            data.Serialize("required", required);
        }

        void AddConnected(Pin* pin) { connectedPins.push_back(pin); }
        void RemoveConnected(Pin* pin)
        {
            auto it = std::find(connectedPins.begin(), connectedPins.end(), pin);
            connectedPins.erase(it);
        }

        void PropagatePending();

        void SetChanged()
        {
            inputState = InputState::Changed;
            PropagatePending();
        }
    };

    class Node : public ISerializeable
    {
      public:
        NE::NodeId id;
        std::string name;
        std::vector<Pin> inputs;
        std::vector<Pin> outputs;
        NodeEditor* editor;

        bool processed = false;
        bool deletable = true;

        Node() = default;
        Node(NE::NodeId id, const std::string& name) : id(id), name(name) {}

        virtual void SetupNode() {};
        virtual void Process() {}
        virtual void Draw();
        virtual void DrawFields() {};

        void DoProcess()
        {
            if (!InputsReady()) return;
            processed = true;
            Process();
            if (processed)
            {
                for (auto& pin : inputs)
                {
                    pin.inputState = InputState::Unchanged;
                }
                for (auto& pin : outputs)
                {
                    for (auto con : pin.connectedPins)
                    {
                        con->SetChanged();
                    }
                }
            }
        }

        void Serialize(DataObject& data) override 
        { 
            if (data.mode == DataObject::DataMode::Inspector) return;

            SerializeId(data, "id", id);
            data.Serialize("name", name);
            data.Serialize("inputs", inputs);
            data.Serialize("outputs", outputs);
        }

        bool InputConnected(uint32_t index) { return !inputs[index].connectedPins.empty(); }

        template<class T, size_t... i>
        void AddInputs(const std::vector<InputPinInfo>& pins);

        template<class T, size_t... i>
        void AddOutputs(const std::vector<OutputPinInfo>& pins);

        template<class T, size_t... i>
        bool IsInputType(uint32_t index);        

        template<class T, size_t... i>
        bool IsInputDataType(uint32_t index);

        template<class T, size_t... i>
        void SetOutputType(uint32_t index);

        bool InputsReady();

        uint32_t GetInputDataType(uint32_t index)
        {
            if (!InputConnected(index)) return 0;
            return inputs[index].connectedPins[0]->type;
        }

        template<class T>
        const T& GetInputData(uint32_t index, const T& defaultValue)
        {
            T* ptr = nullptr;
            if (!inputs[index].connectedPins.empty()) ptr = reinterpret_cast<T*>(inputs[index].connectedPins[0]->outputData);

            return ptr ? *ptr : defaultValue;
        }

        template<class T>
        const T& GetInputData(uint32_t index)
        {
            EVA_INTERNAL_ASSERT(!inputs[index].connectedPins.empty(), "Required pin is not connected");
            return *reinterpret_cast<T*>(inputs[index].connectedPins[0]->outputData);
        }

        template<class T>
        const T* GetInputDataPtr(uint32_t index)
        {
            EVA_INTERNAL_ASSERT(!inputs[index].connectedPins.empty(), "Required pin is not connected");
            return reinterpret_cast<T*>(inputs[index].connectedPins[0]->outputData);
        }

        template<class T>
        void SetOutputData(uint32_t index, T* pointer)
        {
            outputs[index].outputData = reinterpret_cast<void*>(pointer);
        }
    };

    void Pin::PropagatePending()
    {
        node->processed = false;
        for (auto& out : node->outputs)
        {
            for (auto& con : out.connectedPins)
            {
                con->inputState = InputState::Pending;
                con->PropagatePending();
            }
        }
    }

    struct Link
    {
        NE::LinkId id;
        Pin* input;
        Pin* output;

        Link() = default;
        Link(NE::LinkId id, Pin* in, Pin* out) : id(id), input(in), output(out) {}
    };
    struct LinkIdHasher
    {
        std::size_t operator()(const NE::LinkId& k) const { return std::hash<uintptr_t>()(k.Get()); }
    };

    struct NodeEditorStyle
    {
        ImColor linkColor {1.0f, 1.0f, 1.0f};
        ImColor invalidLinkColor {1.0f, 0.0f, 0.0f};

        std::unordered_map<InputState, ImVec4> pinStateColors = {{InputState::Unchanged, {1.0f, 1.0f, 1.0f, 0.0f}},
                                                                    {InputState::Changed, {1.0f, 1.0f, 0.0f, 1.0f}},
                                                                    {InputState::Pending, {1.0f, 0.0f, 0.0f, 1.0f}}};

        ImColor pinColor {1.0f, 1.0f, 1.0f};
        std::unordered_map<uint32_t, ImColor> pinColors;

        template<class T, size_t... i>
        void SetPinColor(const ImColor& color);

        ImColor GetPinColor(uint32_t pinType) 
        { 
            auto it = pinColors.find(pinType);
            return (it == pinColors.end()) ? pinColor : (*it).second;
        }
    };

    class NodeEditor
    {
      public:
        NodeEditor();
        ~NodeEditor();

        void Draw();
        void DrawPin(const Pin& pin);

        template<class T, typename... Args>
        void AddNode(Args&&... args, glm::vec2 position = {})
        {
            auto node = CreateRef<T>(std::forward<Args>(args)...);
           
            AddNode(node, position);
        }

        void AddNode(Ref<Node> node, glm::vec2 position = {})
        {
            m_Nodes.push_back(node);

            node->id     = NextNodeId();
            node->editor = this;
            node->SetupNode();
            node->DoProcess();

            NE::SetCurrentEditor(m_Context);
            NE::SetNodePosition(node->id, {position.x, position.y});
        }

        inline NE::NodeId NextNodeId() { return NE::NodeId(m_NextId++); }
        inline NE::PinId NextPinId() { return NE::PinId(m_NextId++); }
        inline NE::LinkId NextLinkId() { return NE::LinkId(m_NextId++); }

        Node* GetNode(const NE::NodeId& nodeId);
        Node* GetNode(const NE::PinId& pinId);
        Pin* GetPin(const NE::PinId& pinId);

        std::vector<NE::LinkId> GetLinksWithInput(const NE::PinId& pinId);
        std::vector<NE::LinkId> GetLinksWithOutput(const NE::PinId& pinId);

        template<class T, size_t... i>
        static uint32_t GetPinType()
        {
            static uint32_t type = s_PinTypeCounter++;
            return type;
        }

        const std::vector<Node*> GetSelectedNodes() const { return m_SelectedNodes; }

        void AddCompatiblePinType(uint32_t input, uint32_t output) { m_CompatiblePinTypes[input].insert(output); }

        bool IsPinsCompatible(uint32_t input, uint32_t output)
        { 
            return (input == output) || (m_CompatiblePinTypes[input].find(output) != m_CompatiblePinTypes[input].end());
        }

        NodeEditorStyle& GetStyle() { return m_Style; }

      private:
        inline static uint32_t s_PinTypeCounter = 1;

        NE::EditorContext* m_Context;

        std::vector<Ref<Node>> m_Nodes;
        std::vector<Node*> m_ProcessQueue;

        std::unordered_map<NE::LinkId, Link, LinkIdHasher> m_Links;
        uintptr_t m_NextId = 1;

        std::vector<Node*> m_SelectedNodes;

        std::unordered_map<uint32_t, std::unordered_set<uint32_t>> m_CompatiblePinTypes;

        NodeEditorStyle m_Style;
    };

    NodeEditor::NodeEditor()
    {
        NE::Config config;
        config.SettingsFile = "NodeEditor.json";
        m_Context           = NE::CreateEditor(&config);
    }

    NodeEditor::~NodeEditor() { DestroyEditor(m_Context); }

    template<class T, size_t... i>
    void NodeEditorStyle::SetPinColor(const ImColor& color)
    {
        pinColors[NodeEditor::GetPinType<T, i...>()] = color;
    }

    template<class T, size_t... i>
    void Node::AddInputs(const std::vector<InputPinInfo>& pins)
    {
        auto type = NodeEditor::GetPinType<T, i...>();
        for (const auto& pin : pins)
        {
            inputs.push_back(Pin(editor->NextPinId(), PinKind::Input, type, this, pin.name, pin.required));
        }
    }

    template<class T, size_t... i>
    void Node::AddOutputs(const std::vector<OutputPinInfo>& pins)
    {
        auto type = NodeEditor::GetPinType<T, i...>();
        for (const auto& pin : pins)
        {
            Pin p(editor->NextPinId(), PinKind::Output, type, this, pin.name);
            p.outputData = pin.data;
            outputs.push_back(p);
        }
    }

    template<class T, size_t... i>
    bool Node::IsInputType(uint32_t index)
    {
        return inputs[index].type == NodeEditor::GetPinType<T, i...>();
    }

    template<class T, size_t... i>
    bool Node::IsInputDataType(uint32_t index)
    {
        if (!InputConnected(index)) return false;

        return inputs[index].connectedPins[0]->type == NodeEditor::GetPinType<T, i...>();
    }

    template<class T, size_t... i>
    void Node::SetOutputType(uint32_t index)
    {
        outputs[index].type = NodeEditor::GetPinType<T, i...>();
    }

    bool Node::InputsReady()
    {
        for (const auto& pin : inputs)
        {
            if (pin.inputState == InputState::Pending) { return false; }
            if (pin.required && pin.connectedPins.empty()) { return false; }

            for (auto& con : pin.connectedPins)
            {
                if (!con->node->processed) return false;
                if (!editor->IsPinsCompatible(pin.type, con->type)) return false; 
            }
        }
        return true;
    }

    void Node::Draw()
    {
        for (const auto& pin : inputs)
        {
            editor->DrawPin(pin);
        }
        DrawFields();
        for (const auto& pin : outputs)
        {
            editor->DrawPin(pin);
        }
    }

    void NodeEditor::Draw()
    {
        NE::SetCurrentEditor(m_Context);
        NE::Begin("NodeEditor");

        //
        // Draw nodes
        //
        for (auto& node : m_Nodes)
        {
            float width = 150;

            NE::BeginNode(node->id);
            ImGui::PushID(&node);
            ImGui::BeginColumns("col", 1, ImGuiColumnsFlags_NoBorder | ImGuiColumnsFlags_NoResize);
            ImGui::SetColumnWidth(0, width);
            ImGui::PushItemWidth(width * 0.5f);

            auto tSize = ImGui::CalcTextSize(node->name.c_str());
            auto pad   = ImGui::GetColumnWidth() - tSize.x;
            ImGui::Dummy({pad * 0.5f, 0});
            ImGui::SameLine();
            ImGui::Text(node->name.c_str());

            node->Draw();

            ImGui::PopItemWidth();
            ImGui::EndColumns();
            ImGui::Dummy({width, 0});
            ImGui::PopID();
            NE::EndNode();
        }
        
        //
        // Draw Links
        //
        for (const auto& [id, linkInfo] : m_Links)
        {
            ImColor& color = (IsPinsCompatible(linkInfo.input->type, linkInfo.output->type) ? m_Style.linkColor : m_Style.invalidLinkColor);
            NE::Link(id, linkInfo.input->id, linkInfo.output->id, color);
        }

        //
        // Handle creation, returns true if editor want to create a new object
        //
        if (NE::BeginCreate())
        {
            auto showLabel = [](const char* label, ImColor color) {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
                auto size = ImGui::CalcTextSize(label);

                auto padding = ImGui::GetStyle().FramePadding;
                auto spacing = ImGui::GetStyle().ItemSpacing;

                ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

                auto rectMin = ImGui::GetCursorScreenPos() - padding;
                auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

                auto drawList = ImGui::GetWindowDrawList();
                drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
                ImGui::TextUnformatted(label);
            };

            NE::PinId inputPinId, outputPinId;
            if (NE::QueryNewLink(&inputPinId, &outputPinId))
            {
                if (inputPinId && outputPinId)
                {
                    auto in  = GetPin(inputPinId);
                    auto out = GetPin(outputPinId);
                    if (in == nullptr || out == nullptr || in == out) { NE::RejectNewItem(); }
                    else
                    {
                        if (in->kind == NE::PinKind::Output)
                        {
                            std::swap(inputPinId, outputPinId);
                            std::swap(in, out);
                        }

                        if (in->node->id == out->node->id)
                        {
                            showLabel("Cannot connect to self", ImColor(150, 50, 50));
                            NE::RejectNewItem();
                        }
                        else if (in->kind == out->kind || !IsPinsCompatible(in->type, out->type))
                        {
                            showLabel("Incompatible pin", ImColor(150, 50, 50));
                            NE::RejectNewItem();
                        }
                        else
                        {
                            showLabel("Create Link", ImColor(50, 150, 50));
                            if (NE::AcceptNewItem())
                            {
                                auto links = GetLinksWithInput(inputPinId);
                                for (const auto& lId : links)
                                {
                                    auto& link = m_Links[lId];

                                    link.output->RemoveConnected(link.input);

                                    link.input->RemoveConnected(link.output);
                                    // link.input->SetChanged();

                                    m_Links.erase(lId);
                                }

                                out->AddConnected(in);

                                in->AddConnected(out);
                                in->SetChanged();
                                if (!out->node->processed) in->inputState = InputState::Pending;

                                auto id = NextLinkId();
                                m_Links.emplace(id, Link {id, in, out});
                            }
                        }
                    }
                }
            }
        }
        NE::EndCreate();

        //
        // Handle deletion
        //
        if (NE::BeginDelete())
        {
            NE::LinkId deletedLinkId;
            while (NE::QueryDeletedLink(&deletedLinkId))
            {
                if (NE::AcceptDeletedItem())
                {
                    auto& link = m_Links[deletedLinkId];

                    link.output->RemoveConnected(link.input);

                    link.input->RemoveConnected(link.output);
                    link.input->SetChanged();

                    m_Links.erase(deletedLinkId);
                }
                // NE::RejectDeletedItem();
            }

            NE::NodeId nodeId = 0;
            while (NE::QueryDeletedNode(&nodeId))
            {
                auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(), [nodeId](const Ref<Node>& node) { return node->id == nodeId; });
                if (it != m_Nodes.end()) 
                { 
                    if ((*it)->deletable) {
                        if (NE::AcceptDeletedItem())
                        {
                            m_Nodes.erase(it);
                        }
                    }
                    else
                    {
                        NE::RejectDeletedItem();
                        NE::SelectNode(nodeId, true);
                    }
                }
            }
        }
        NE::EndDelete();

        //
        // Get selected
        //
        std::vector<NE::NodeId> nodeIds(NE::GetSelectedObjectCount());
        int count = NE::GetSelectedNodes(nodeIds.data(), nodeIds.size());
        m_SelectedNodes.clear();
        for (int i = 0; i < count; i++)
        {
            m_SelectedNodes.push_back(GetNode(nodeIds[i]));
        }

        //
        // Process
        //
        for (const auto node : m_Nodes) 
        {
            if (!node->processed) 
            { 
                node->DoProcess();
                if (node->processed) break;
            }
        }


        //
        // End of interaction with editor.
        //
        NE::End();
        NE::SetCurrentEditor(nullptr);
    }

    void NodeEditor::DrawPin(const Pin& pin)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        NE::BeginPin(pin.id, pin.kind);

        auto col     = ImGui::GetColumnWidth();
        auto tSize   = ImGui::CalcTextSize(pin.name.c_str());
        float h      = tSize.y * 0.5f;
        float radius = tSize.y * 0.4f;
        float border = 15.0f;

        if (pin.kind == NE::PinKind::Input)
        {
            NE::PinPivotAlignment({0, 0.5f});

            auto pos   = ImGui::GetCursorScreenPos();

            auto color = m_Style.GetPinColor(pin.type);
            drawList->AddCircleFilled(pos + ImVec2(0, h), radius, color);

            auto outlineColor = ImColor(m_Style.pinStateColors.at(pin.inputState));
            drawList->AddCircle(pos + ImVec2(0, h), radius, outlineColor);

            ImGui::Dummy(ImVec2(radius, 0));
            ImGui::SameLine();
            ImGui::Text(pin.name.c_str());
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(col - radius - tSize.x - border, 0));
        }
        else
        {
            NE::PinPivotAlignment({1, 0.5f});

            auto dummy = col - tSize.x - radius - border;

            ImGui::Dummy({dummy, 0});
            ImGui::SameLine();

            ImGui::Text(pin.name.c_str());
            ImGui::SameLine();

            auto pos = ImGui::GetCursorScreenPos();
            auto color = m_Style.GetPinColor(pin.type);
            drawList->AddCircleFilled(pos + ImVec2(0.0f, h), radius, color);
            ImGui::Dummy({radius, 0});
        }
        NE::EndPin();
    }

    Node* NodeEditor::GetNode(const NE::NodeId& nodeId)
    {
        auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(), [&](const Ref<Node>& n) { return n->id == nodeId; });
        return it == m_Nodes.end() ? nullptr : (*it).get();
    }

    Node* NodeEditor::GetNode(const NE::PinId& pinId)
    {
        for (auto& n : m_Nodes)
        {
            for (auto& p : n->inputs)
                if (p.id == pinId) return n.get();

            for (auto& p : n->outputs)
                if (p.id == pinId) return n.get();
        }
        return nullptr;
    }

    Pin* NodeEditor::GetPin(const NE::PinId& pinId)
    {
        for (auto& n : m_Nodes)
        {
            for (auto& p : n->inputs)
                if (p.id == pinId) return &p;

            for (auto& p : n->outputs)
                if (p.id == pinId) return &p;
        }
        return nullptr;
    }

    std::vector<NE::LinkId> NodeEditor::GetLinksWithInput(const NE::PinId& pinId)
    {
        std::vector<NE::LinkId> links;
        for (const auto& [lid, link] : m_Links)
        {
            if (link.input->id == pinId) links.push_back(link.id);
        }
        return links;
    }

    std::vector<NE::LinkId> NodeEditor::GetLinksWithOutput(const NE::PinId& pinId)
    {
        std::vector<NE::LinkId> links;
        for (const auto& [lid, link] : m_Links)
        {
            if (link.output->id == pinId) links.push_back(link.id);
        }
        return links;
    }
} // namespace EVA::NE
