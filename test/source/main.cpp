﻿#include "EVA.hpp"

class ExampleLayer : public EVA::Layer
{
public:
	ExampleLayer() : Layer("Example") {}

	void OnUpdate() override
	{
		if(EVA::Input::IsKeyPressed(EVA::KeyCode::Tab))
			EVA_TRACE("Example layer: TAB");
	}

	void OnEvent(EVA::Event& e) override
	{
		//EVA_INFO("Example layer: {0}", e);
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Example window");
		ImGui::Text("Hello world");
		ImGui::End();
	}
};

int main()
{
	EVA::Application app;
	app.PushLayer(new ExampleLayer());
	app.Run();

	return 0;
}
