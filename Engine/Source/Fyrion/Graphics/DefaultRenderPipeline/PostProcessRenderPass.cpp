#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"

namespace Fyrion
{
    struct PostProcessRenderPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState;
        BindingSet* bindingSet;

        void Init() override
        {
            ShaderAsset* shader = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/PostProcessRender.comp");

            pipelineState = Graphics::CreateComputePipelineState({
                .shader = shader
            });

            bindingSet = Graphics::CreateBindingSet(shader);
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            RenderGraphResource* lightColor = node->GetInputResource("LightColor");
            RenderGraphResource* outputColor = node->GetOutputResource("OutputColor");

            bindingSet->GetVar("inputTexture")->SetTexture(lightColor->texture);
            bindingSet->GetVar("outputTexture")->SetTexture(outputColor->texture);

            cmd.BindPipelineState(pipelineState);
            cmd.BindBindingSet(pipelineState, bindingSet);

            cmd.Dispatch(std::ceil(lightColor->textureCreation.extent.width / 8.f),
                         std::ceil(lightColor->textureCreation.extent.height / 8.f),
                         1.f);
        }

        void Destroy() override
        {
            Graphics::DestroyBindingSet(bindingSet);
            Graphics::DestroyComputePipelineState(pipelineState);
        }

        static void RegisterType(NativeTypeHandler<PostProcessRenderPass>& type)
        {
            RenderGraphPassBuilder<PostProcessRenderPass>::Builder(RenderGraphPassType::Compute)
                .Input(RenderGraphResourceCreation{
                    .name = "LightColor",
                    .type = RenderGraphResourceType::Texture,
                    .format = Format::RGBA16F
                })
                .Output(RenderGraphResourceCreation{
                    .name = "OutputColor",
                    .type = RenderGraphResourceType::Texture,
                    .scale = {1.0, 1.0},
                    .format = Format::RGBA16F
                });
        }
    };

    void RegisterPostProcessRenderPass()
    {
        Registry::Type<PostProcessRenderPass>();
    }
}
