#pragma once

#include "LayerStack.hpp"
#include "Window.hpp"
#include "Events/Window.hpp"
#include "ImGui/ImGuiLayer.hpp"
namespace EVA
{
    class Application
    {
        inline static Application* s_Instance = nullptr;

        std::unique_ptr<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        bool m_Running = true;

        LayerStack m_LayerStack;

    public:

        Application();
        ~Application();

        void Run();
        void Exit();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        inline Window& GetWindow() { return *m_Window.get(); }

        inline static Application& Get() { return *s_Instance; }

    private:
        void OnEvent(Event& event);
        bool OnWindowClosed(WindowCloseEvent& event);
    };
}
