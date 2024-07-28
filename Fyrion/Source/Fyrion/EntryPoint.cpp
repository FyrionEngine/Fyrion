#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/ArgParser.hpp"
#include "Fyrion/Core/Sinks.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Editor/Launcher/Launcher.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"

using namespace Fyrion;

int main(i32 argc, char** argv)
{
    RenderGraph::SetRegisterSwapchainRenderEvent(false);
    StdOutSink stdOutSink{};
    Logger::RegisterSink(stdOutSink);

    ArgParser args{};
    args.Parse(argc, argv);

    String projectPath = args.Get("projectPath");

    if (projectPath.Empty())
    {
        Engine::Init(argc, argv);
        Launcher::Init();
        Engine::Run();
        projectPath = Launcher::GetSelectedProject();
        Engine::Destroy();
    }

    if (!projectPath.Empty())
    {
        Engine::Init(argc, argv);
        Editor::Init();

        EngineContextCreation contextCreation{
            .title = "Fyrion Engine",
            .resolution = {1920, 1080},
            .maximize = true,
            .headless = false,
        };

        Engine::CreateContext(contextCreation);

        Engine::Run();
        Engine::Destroy();
    }

    return 0;
}