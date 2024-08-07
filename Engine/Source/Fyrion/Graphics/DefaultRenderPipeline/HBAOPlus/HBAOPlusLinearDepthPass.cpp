#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"

namespace Fyrion::HBAOPlus
{
    struct LinearDepthPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        void Init() override
        {

            ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/HBAOPlus/LinearizeDepth.comp");

            Graphics::CreateComputePipelineState({
                .shader = shaderAsset
            });


        }

        static void RegisterType(NativeTypeHandler<LinearDepthPass>& type)
        {
            RenderGraphPassBuilder<LinearDepthPass>::Builder(RenderGraphPassType::Compute)
                // .Output(RenderGraphResourceCreation{
                //     .name = "ShadowDepthTexture",
                //     .type = RenderGraphResourceType::Reference,
                // })
                ;

        }
    };


    void RegisterLinearDepthPass()
    {
        Registry::Type<LinearDepthPass>();
    }
}
