#include "EVA.hpp"
#include "EVA/Editor/EditorLayer.hpp"
#include "ComputeLayer.hpp"
#include "EVA/Procedural/TextureGeneratorLayer.hpp"
#include "EVA/Assets/AssetHandlers.hpp"

int main()
{
    EVA_PROFILE_BEGIN_SESSION("profiler-startup.json");

	auto app = new EVA::Application(EVA::WindowProperties("Texture graph"));
	app->GetWindow().SetVSync(false);

    EVA::AssetManager::Register<EVA::Shader>(".glsl", false);
    EVA::AssetManager::Register<EVA::Mesh>(".obj", false);

    EVA::AssetManager::Register<EVA::Texture>(".png", false);
    EVA::AssetManager::Register<EVA::Texture>(".hdr", false);

    EVA::AssetManager::Register<EVA::NE::NodeGraph>(".graph", true);
    EVA::FileDialog::Register(".graph", "Node graph");

    //app->PushLayer(new EVA::EditorLayer());
    //app->PushLayer(new EVA::ComputeLayer());
    app->PushLayer(new EVA::TextureGeneratorLayer());

    EVA_PROFILE_END_SESSION();


    EVA_PROFILE_BEGIN_SESSION("profiler-runtime.json");
	app->Run();
    EVA_PROFILE_END_SESSION();


    EVA_PROFILE_BEGIN_SESSION("profiler-shutdown.json");
	delete app;
    EVA_PROFILE_END_SESSION();

	return 0;
}
