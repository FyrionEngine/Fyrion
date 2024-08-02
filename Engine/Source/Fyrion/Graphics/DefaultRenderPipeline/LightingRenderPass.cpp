#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"

namespace Fyrion
{
    struct LightingData
    {
        Mat4 viewInverse{};
        Mat4 projInverse{};
    };

    class LightingRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState lightingPSO{};
        BindingSet*   bindingSet{};

        Texture skyTexture{};

        void Init() override
        {
            ComputePipelineCreation creation{
                .shader = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Lighting.comp")
            };

            lightingPSO = Graphics::CreateComputePipelineState(creation);
            bindingSet = Graphics::CreateBindingSet(creation.shader);

            skyTexture = AssetDatabase::FindByPath<TextureAsset>("Sandbox://CasualDay4K.hdr")->GetTexture();
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            const CameraData& cameraData = graph->GetCameraData();

            LightingData data {
                .viewInverse = cameraData.viewInverse,
                .projInverse = cameraData.projectionInverse,
            };


            RenderGraphResource* gbufferColor = node->GetInputResource("GBufferColor");
            RenderGraphResource* lightColor = node->GetOutputResource("LightColor");

            bindingSet->GetVar("gbufferColor")->SetTexture(gbufferColor->texture);
            bindingSet->GetVar("depthTex")->SetTexture(node->GetInputTexture("Depth"));
            bindingSet->GetVar("lightColor")->SetTexture(lightColor->texture);
            bindingSet->GetVar("skyCubeTex")->SetTexture(skyTexture);
            bindingSet->GetVar("data")->SetValue(&data, sizeof(LightingData));

            cmd.BindPipelineState(lightingPSO);
            cmd.BindBindingSet(lightingPSO, bindingSet);

            //TODO : remove this barrier, render graph should manage it automatically
            ResourceBarrierInfo resourceBarrierInfo{};
            resourceBarrierInfo.texture = lightColor->texture;
            resourceBarrierInfo.oldLayout = ResourceLayout::Undefined;
            resourceBarrierInfo.newLayout = ResourceLayout::General;
            cmd.ResourceBarrier(resourceBarrierInfo);

            cmd.Dispatch(std::ceil(gbufferColor->textureCreation.extent.width / 16.f),
                         std::ceil(gbufferColor->textureCreation.extent.height / 16.f),
                         1.f);

            //TODO : remove this barrier, render graph should manage it automatically
            resourceBarrierInfo.oldLayout = ResourceLayout::General;
            resourceBarrierInfo.newLayout = ResourceLayout::ShaderReadOnly;
            cmd.ResourceBarrier(resourceBarrierInfo);
        }

        void Destroy() override
        {
            Graphics::DestroyBindingSet(bindingSet);
            Graphics::DestroyComputePipelineState(lightingPSO);
        }

        static void RegisterType(NativeTypeHandler<LightingRenderPass>& type)
        {
            RenderGraphPassBuilder<LightingRenderPass>::Builder(RenderGraphPassType::Compute)
                .Input(RenderGraphResourceCreation{
                    .name = "GBufferColor",
                    .type = RenderGraphResourceType::Texture,
                    .format = Format::RGBA
                })
                .Input(RenderGraphResourceCreation{
                    .name = "Depth",
                    .type = RenderGraphResourceType::Texture,
                    .format = Format::Depth
                })
                .Output(RenderGraphResourceCreation{
                    .name = "LightColor",
                    .type = RenderGraphResourceType::Texture,
                    .scale = {1.0, 1.0},
                    .format = Format::RGBA
                });
        }
    };

    void RegisterLightingRenderPass()
    {
        Registry::Type<LightingRenderPass>();
    }
}
