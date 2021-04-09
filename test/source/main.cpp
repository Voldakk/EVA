#include "EVA.hpp"
#include "EVA/Editor/EditorLayer.hpp"
#include "ComputeLayer.hpp"

int main()
{
    EVA_PROFILE_BEGIN_SESSION("profiler-startup.json");

	auto app = new EVA::Application();

    //app->PushLayer(new EVA::EditorLayer());
    app->PushLayer(new EVA::ComputeLayer());

	app->GetWindow().SetVSync(false);

    EVA_PROFILE_END_SESSION();


    EVA_PROFILE_BEGIN_SESSION("profiler-runtime.json");
	app->Run();
    EVA_PROFILE_END_SESSION();


    EVA_PROFILE_BEGIN_SESSION("profiler-shutdown.json");
	delete app;
    EVA_PROFILE_END_SESSION();

	return 0;
}
