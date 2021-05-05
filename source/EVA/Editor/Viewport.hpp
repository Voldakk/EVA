#pragma once

namespace EVA
{
    class Viewport
    {
      public:
        Viewport()
        {
            FramebufferSpecification spec;
            spec.width = 8;
            spec.height = 8;
            Init(spec);
        }
        Viewport(const FramebufferSpecification& spec)
        {
            Init(spec);
        }
        void Init(const FramebufferSpecification& spec)
        {
            EVA_PROFILE_FUNCTION();

            m_Size.x = spec.width;
            m_Size.y = spec.height;
            m_Framebuffer = Framebuffer::Create(spec);
            m_TextureId = m_Framebuffer->GetColorAttachmentRendererId();
        }
        ~Viewport() = default;

        bool Update()
        {
            EVA_PROFILE_FUNCTION();

            bool changed = false;
            if (m_Resize)
            {
                changed  = true;
                m_Resize = false;
                m_Framebuffer->Resize((uint32_t)m_Size.x, (uint32_t)m_Size.y);
                m_TextureId = m_Framebuffer->GetColorAttachmentRendererId();
            }

            return changed;
        }

        void Draw()
        {
            EVA_PROFILE_FUNCTION();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 {0, 0});
            ImGui::Begin("Viewport");

            m_Focused = ImGui::IsWindowFocused();
            m_Hovered = ImGui::IsWindowHovered();

            Application::Get().GetImGuiLayer()->BlockEvents(!m_Focused || !m_Hovered);

            auto panelSize = ImGui::GetContentRegionAvail();
            if (m_Size != *reinterpret_cast<glm::vec2*>(&panelSize) && panelSize.x > 0 && panelSize.y > 0)
            {
                m_Size   = {panelSize.x, panelSize.y};
                m_Resize = true;
            }
            ImGui::Image(*reinterpret_cast<void**>(&m_TextureId), panelSize, ImVec2 {0.0f, 1.0f}, ImVec2 {1.0f, 0.0f});

            ImGui::End();
            ImGui::PopStyleVar(ImGuiStyleVar_WindowPadding);
        }

        void Bind() { m_Framebuffer->Bind(); }
        void Unbind() { m_Framebuffer->Unbind(); }

        Ref<Framebuffer> GetFramebuffer() { return m_Framebuffer; }

        glm::vec2 GetSize() const { return m_Size; }
        bool IsFocused() const { return m_Focused; }
        bool IsHovered() const { return m_Hovered; }

        void SetTexture(uint32_t id) {m_TextureId = id;}

      private:
        glm::vec2 m_Size = {0.0f, 0.0f};
        Ref<Framebuffer> m_Framebuffer;

        bool m_Focused = false;
        bool m_Hovered = false;
        bool m_Resize  = false;
        uint32_t m_TextureId;
    };
}
