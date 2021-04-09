#include "EVA.hpp"
#include "EVA/Editor/EditorLayer.hpp"
#include "ComputeLayer.hpp"

int main()
{
	EVA::Application app;
    //app.PushLayer(new EVA::EditorLayer());
    app.PushLayer(new EVA::ComputeLayer());
	app.GetWindow().SetVSync(false);
	app.Run();

	return 0;
}
