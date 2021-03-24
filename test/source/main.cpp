#include "EVA.hpp"
#include "EVA/Editor/EditorLayer.hpp"

int main()
{
	EVA::Application app;
	app.PushLayer(new EVA::EditorLayer());
	app.GetWindow().SetVSync(false);
	app.Run();

	return 0;
}
