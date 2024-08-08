#include "HBAOPlus.hpp"

#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"


namespace Fyrion::HBAOPlus
{
    struct LinearDepthPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState{};

        void Init() override
        {
            ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/HBAOPlus/LinearizeDepth.comp");

            pipelineState = Graphics::CreateComputePipelineState({
                .shader = shaderAsset
            });
        }

        void Destroy() override
        {
            Graphics::DestroyComputePipelineState(pipelineState);
        }

        static void RegisterType(NativeTypeHandler<LinearDepthPass>& type)
        {
            RenderGraphPassBuilder<LinearDepthPass>::Builder(RenderGraphPassType::Compute);
        }
    };


    struct DeinterleaveDepthPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState{};

        void Init() override
        {
            ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/HBAOPlus/DeinterleaveDepth.comp");

            pipelineState = Graphics::CreateComputePipelineState({
                .shader = shaderAsset
            });
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override {}

        void Destroy() override {}


        static void RegisterType(NativeTypeHandler<DeinterleaveDepthPass>& type)
        {
            RenderGraphPassBuilder<DeinterleaveDepthPass>::Builder(RenderGraphPassType::Compute);
        }
    };

    struct CoarseAOPSPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState{};

        void Init() override
        {
            ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/HBAOPlus/CoarseAO.raster");

            pipelineState = Graphics::CreateGraphicsPipelineState({
                .shader = shaderAsset,
                .depthFormat = Format::Depth
            });
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override {}

        void Destroy() override {}

        static void RegisterType(NativeTypeHandler<CoarseAOPSPass>& type)
        {
            RenderGraphPassBuilder<CoarseAOPSPass>::Builder(RenderGraphPassType::Other);
        }
    };

    struct ReinterleavedAOPSPreBlurPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState{};

        void Init() override
        {
            ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/HBAOPlus/ReinterleaveAO.comp");

            pipelineState = Graphics::CreateComputePipelineState({
                .shader = shaderAsset
            });
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override {}

        void Destroy() override {}

        static void RegisterType(NativeTypeHandler<ReinterleavedAOPSPreBlurPass>& type)
        {
            RenderGraphPassBuilder<ReinterleavedAOPSPreBlurPass>::Builder(RenderGraphPassType::Compute);
        }
    };

    struct BlurPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState{};

        void Init() override
        {

            {
                ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/HBAOPlus/BlurX.comp");

                pipelineState = Graphics::CreateComputePipelineState({
                    .shader = shaderAsset
                });
            }


            {
                ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/HBAOPlus/BlurY.comp");

                pipelineState = Graphics::CreateComputePipelineState({
                    .shader = shaderAsset
                });
            }
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override {}

        void Destroy() override {}

        static void RegisterType(NativeTypeHandler<BlurPass>& type)
        {
            RenderGraphPassBuilder<BlurPass>::Builder(RenderGraphPassType::Compute);
        }
    };
}

namespace Fyrion
{
    void RegisterHBAOPlus()
    {
        Registry::Type<HBAOPlus::LinearDepthPass>();
        Registry::Type<HBAOPlus::DeinterleaveDepthPass>();
        Registry::Type<HBAOPlus::CoarseAOPSPass>();
        Registry::Type<HBAOPlus::ReinterleavedAOPSPreBlurPass>();
        Registry::Type<HBAOPlus::BlurPass>();
    }
}
