#pragma once

#include <imgui_node_editor.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

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

    inline static const std::unordered_map<InputState, ImVec4> s_InputStateColor = {{InputState::Unchanged, {1.0f, 1.0f, 1.0f, 1.0f}},
                                                                                    {InputState::Changed, {1.0f, 1.0f, 0.0f, 1.0f}},
                                                                                    {InputState::Pending, {1.0f, 0.0f, 0.0f, 1.0f}}};

    class NodeEditor;
    struct Node;

    struct PinInfo
    {
        NE::PinKind kind;
        uint32_t type;
        std::string name;
        bool required = true;
    };

    struct Pin
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

    struct Node
    {
        NE::NodeId id;
        std::string name;
        std::vector<Pin> inputs;
        std::vector<Pin> outputs;
        NodeEditor* editor;

        bool processed = false;

        Node() = default;
        Node(NE::NodeId id, const std::string& name) : id(id), name(name) {}

        virtual void SetupNode() {};
        virtual void Process() {}
        virtual void Draw();
        virtual void DrawFields() {};

        void DoProcess()
        {
            if (!InputsReady()) return;
            Process();
            processed = true;
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

        bool InputConnected(uint32_t index) { return !inputs[index].connectedPins.empty(); }

        void AddPins(const std::vector<PinInfo>& pins);
        bool InputsReady()
        {
            for (const auto& pin : inputs)
            {
                if (pin.inputState == InputState::Pending) { return false; }
                if (pin.required && pin.connectedPins.empty()) { return false; }

                for (auto& con : pin.connectedPins)
                {
                    if (!con->node->processed) return false;
                }
            }
            return true;
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

    class NodeEditor
    {
      public:
        NodeEditor();
        ~NodeEditor();

        void Draw();
        void DrawPin(const Pin& pin);

        template<class T, typename... Args>
        void AddNode(Args&&... args)
        {
            auto nId = NextNodeId();

            auto node = CreateRef<T>(std::forward<Args>(args)...);
            m_Nodes.push_back(node);

            node->id     = NextNodeId();
            node->editor = this;
            node->SetupNode();
            node->DoProcess();
        }

        inline NE::NodeId NextNodeId() { return NE::NodeId(m_NextId++); }
        inline NE::PinId NextPinId() { return NE::PinId(m_NextId++); }
        inline NE::LinkId NextLinkId() { return NE::LinkId(m_NextId++); }

        Node* GetNode(const NE::NodeId& nodeId);
        Node* GetNode(const NE::PinId& pinId);
        Pin* GetPin(const NE::PinId& pinId);

        std::vector<NE::LinkId> GetLinksWithInput(const NE::PinId& pinId);
        std::vector<NE::LinkId> GetLinksWithOutput(const NE::PinId& pinId);

        template<class T>
        static uint32_t GetPinType()
        {
            static uint32_t type = s_PinTypeCounter++;
            return type;
        }

      private:
        inline static uint32_t s_PinTypeCounter = 1;

        NE::EditorContext* m_Context;

        std::vector<Ref<Node>> m_Nodes;
        std::vector<Node*> m_ProcessQueue;

        std::unordered_map<NE::LinkId, Link, LinkIdHasher> m_Links;
        uintptr_t m_NextId = 1;
    };

    NodeEditor::NodeEditor()
    {
        NE::Config config;
        config.SettingsFile = "NodeEditor.json";
        m_Context           = NE::CreateEditor(&config);
    }

    NodeEditor::~NodeEditor() { DestroyEditor(m_Context); }


    void Node::AddPins(const std::vector<PinInfo>& pins)
    {
        for (const auto& pin : pins)
        {
            std::vector<Pin>& v = (pin.kind == NE::PinKind::Input ? inputs : outputs);
            v.push_back(Pin(editor->NextPinId(), pin.kind, pin.type, this, pin.name, pin.required));
        }
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
        NE::Begin("My Editor", ImVec2(0.0, 0.0f));

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

            if (node->processed) { ImGui::Text("Processed"); }
            else if (ImGui::Button("Process"))
            {
                node->DoProcess();
            }

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
            NE::Link(id, linkInfo.input->id, linkInfo.output->id);
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
                        else if (in->kind == out->kind || in->type != out->type)
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
                if (NE::AcceptDeletedItem())
                {
                    auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(), [nodeId](const Ref<Node>& node) { return node->id == nodeId; });
                    if (it != m_Nodes.end()) m_Nodes.erase(it);
                }
                // NE::RejectDeletedItem();
            }
        }
        NE::EndDelete();

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
            auto color = ImColor(s_InputStateColor.at(pin.inputState));
            drawList->AddCircleFilled(pos + ImVec2(0, h), radius, color);
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
            drawList->AddCircleFilled(pos + ImVec2(0.0f, h), radius, ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
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
