
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Sinks.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceAssets.hpp"

using namespace Fyrion;

PipelineState graphicsPipeline = {};

void Draw(RenderCommands& renderCommands)
{
    renderCommands.BindPipelineState(graphicsPipeline);
    renderCommands.Draw(3, 1, 0, 0);
}

void Shutdown()
{
    Graphics::DestroyGraphicsPipelineState(graphicsPipeline);
}

int main(i32 argc, char** argv)
{
    StdOutSink stdOutSink{};
    Logger::RegisterSink(stdOutSink);

    Engine::Init(argc, argv);

    Event::Bind<OnSwapchainRender, Draw>();
    Event::Bind<OnShutdown, Shutdown>();

    EngineContextCreation contextCreation{
        .title = "Fyrion Engine",
        .resolution = {1920, 1080},
        .maximize = true,
        .headless = false,
    };
    Engine::CreateContext(contextCreation);

    ResourceAssets::LoadAssetsFromDirectory("SandboxTest", "C:\\dev\\FyrionEngine\\Projects\\SandboxTest\\Assets");

    Format format = Format::BGRA;

    GraphicsPipelineCreation graphicsPipelineCreation{
        .shader = Repository::GetByPath("SandboxTest://TestRaster.raster"),
        .attachments = &format
    };

    graphicsPipeline = Graphics::CreateGraphicsPipelineState(graphicsPipelineCreation);

    Engine::Run();
    Engine::Destroy();

    return 0;
}