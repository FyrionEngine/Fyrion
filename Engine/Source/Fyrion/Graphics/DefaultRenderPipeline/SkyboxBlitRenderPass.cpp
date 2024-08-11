#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"

namespace Fyrion
{
    class SkyboxBlitRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState = {};
        BindingSet* bindingSet = {};


        struct SkyboxRenderData
        {
            Mat4 viewInverse;
            Mat4 projInverse;
            Vec4 skyboxProperties; //rgb color, a = hasSkyboxTexture.
        };

        void Init() override
        {
            ShaderAsset* shaderAsset = AssetManager::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/SkyboxRender.comp");

            pipelineState = Graphics::CreateComputePipelineState({
                .shader = shaderAsset
            });

            bindingSet = Graphics::CreateBindingSet(shaderAsset);
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            const CameraData& cameraData = graph->GetCameraData();

            RenderGraphResource* skybox = node->GetInputResource("Skybox");
            RenderGraphResource* lightColor = node->GetInputResource("LightColor");
            RenderGraphResource* depthTexture = node->GetInputResource("Depth");

            SkyboxRenderData data{
                .viewInverse = cameraData.viewInverse,
                .projInverse = cameraData.projectionInverse,
                .skyboxProperties = Math::MakeVec4(Color::CORNFLOWER_BLUE.ToVec3(), skybox->reference != nullptr ? 1.0 : 0.0)
            };

            bindingSet->GetVar("skyboxTexture")->SetTexture(skybox->texture);
            bindingSet->GetVar("colorTexture")->SetTexture(lightColor->texture);
            bindingSet->GetVar("depthTexture")->SetTexture(depthTexture->texture);
            bindingSet->GetVar("data")->SetValue(&data, sizeof(data));

            cmd.BindPipelineState(pipelineState);
            cmd.BindBindingSet(pipelineState, bindingSet);

            cmd.Dispatch(std::ceil(lightColor->textureCreation.extent.width / 16.f),
                         std::ceil(lightColor->textureCreation.extent.height / 16.f),
                         1.f);
        }

        void Destroy() override
        {
            Graphics::DestroyComputePipelineState(pipelineState);
            Graphics::DestroyBindingSet(bindingSet);
        }

        static void RegisterType(NativeTypeHandler<SkyboxBlitRenderPass>& type)
        {
            RenderGraphPassBuilder<SkyboxBlitRenderPass>::Builder(RenderGraphPassType::Compute)
                .Input(RenderGraphResourceCreation{
                    .name = "Skybox",
                    .type = RenderGraphResourceType::Reference,
                })
                .Input(RenderGraphResourceCreation{
                    .name = "LightColor",
                    .type = RenderGraphResourceType::Texture,
                    .format = Format::RGBA16F
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
                    .format = Format::RGBA16F
                });
        }
    };

    void RegisterSkyboxBlitRenderPass()
    {
        Registry::Type<SkyboxBlitRenderPass>();
    }
}
