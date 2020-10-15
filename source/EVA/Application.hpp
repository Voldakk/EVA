#pragma once

#include "EVA/LayerStack.hpp"
#include "EVA/Window.hpp"
#include "EVA/Events/Window.hpp"
#include "EVA/ImGui/ImGuiLayer.hpp"
#include "EVA/Renderer/Buffer.hpp"
#include "EVA/Renderer/VertexArray.hpp"
#include "EVA/Renderer/Shader.hpp"

namespace EVA
{
    class Application
    {
        inline static Application* s_Instance = nullptr;

        std::unique_ptr<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;
        bool m_Minimized = false;

        LayerStack m_LayerStack;

    public:

        Application();
        ~Application();

        void Run();
        void Exit();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        [[nodiscard]] inline Window& GetWindow() { return *m_Window.get(); }

        [[nodiscard]] inline static Application& Get() { return *s_Instance; }

    private:
        void OnEvent(Event& e);
        bool OnWindowClosed(WindowCloseEvent& e);
        bool OnWindowResized(WindowResizeEvent& e);
    };
}
