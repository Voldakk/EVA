#include "EVA.hpp"

class ExampleLayer : public EVA::Layer
{
public:
	ExampleLayer() : Layer("Example") {}

	void OnUpdate() override
	{
		EVA_TRACE("Example layer: OnUpdate");
	}

	void OnEvent(EVA::Event& e) override
	{
		EVA_INFO("Example layer: {0}", e);
	}
};

int main()
{
	EVA::Application app;
	app.PushLayer(new ExampleLayer());
	app.Run();

	return 0;
}
