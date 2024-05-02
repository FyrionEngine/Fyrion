
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Sinks.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceAssets.hpp"

using namespace Fyrion;

PipelineState graphicsPipeline = {};
Buffer vertexBuffer = {};

void Draw(RenderCommands& renderCommands)
{
    renderCommands.BindPipelineState(graphicsPipeline);
    renderCommands.BindVertexBuffer(vertexBuffer);
    renderCommands.Draw(3, 1, 0, 0);
}

void Shutdown()
{
    Graphics::DestroyGraphicsPipelineState(graphicsPipeline);
    Graphics::DestroyBuffer(vertexBuffer);
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

    const Vec3 positions[3] = {
        Vec3{0.0f, -0.5f, 0.0f},
        Vec3{0.5, 0.5, 0.0},
        Vec3{-0.5, 0.5, 0.0}
    };

    vertexBuffer = Graphics::CreateBuffer(BufferCreation{
        .usage = BufferUsage::VertexBuffer,
        .size = sizeof(positions),
        .allocation = BufferAllocation::GPUOnly,
    });

    Graphics::UpdateBufferData(BufferDataInfo{
        .buffer = vertexBuffer,
        .data = &positions,
        .size = sizeof(positions),
        .offset = 0
    });

    graphicsPipeline = Graphics::CreateGraphicsPipelineState(graphicsPipelineCreation);

    BindingSet& bindingSet = Graphics::CreateBindingSet(graphicsPipelineCreation.shader, BindingSetType::Dynamic);
    bindingSet["buffer"] = vertexBuffer;


    Engine::Run();
    Engine::Destroy();

    return 0;
}