#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Sinks.hpp"
#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"

using namespace Fyrion;

int main(i32 argc, char** argv)
{
    StdOutSink stdOutSink{};
    Logger::RegisterSink(stdOutSink);

    RenderGraph::SetRegisterSwapchainRenderEvent(false);

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


    return 0;
}