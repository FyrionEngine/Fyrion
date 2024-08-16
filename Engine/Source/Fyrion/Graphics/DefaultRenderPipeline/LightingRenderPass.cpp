#include "DefaultRenderPipelineTypes.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/RenderStorage.hpp"
#include "Fyrion/Graphics/RenderUtils.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"

namespace Fyrion
{
    struct DirectionalLightData
    {
        Vec4             direction;
        Vec4             color;
        alignas(16) Vec2 intensityIndirect;
    };

    struct LightingData
    {
        Vec4                 cascadeSplits;
        Mat4                 cascadeViewProjMat[FY_SHADOW_MAP_CASCADE_COUNT];
        Vec4                 viewPos{};
        Mat4                 view{};
        DirectionalLightData directionalLight[4];
        u32                  lightCount[4] = {};
    };

    class LightingRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState lightingPSO{};
        BindingSet*   bindingSet{};
        VoidPtr       skyboxReference{};

        DiffuseIrradianceGenerator diffuseIrradianceGenerator;
        BRDFLUTGenerator           brdflutGenerator;
        SpecularMapGenerator       specularMapGenerator;

        Sampler                    shadowMapSampler;

        void Init() override
        {
            ComputePipelineCreation creation{
                .shader = AssetManager::LoadByPath<ShaderAsset>("Fyrion://Shaders/Passes/LightingPass.comp")
            };

            lightingPSO = Graphics::CreateComputePipelineState(creation);
            bindingSet = Graphics::CreateBindingSet(creation.shader);

            diffuseIrradianceGenerator.Init({64, 64});
            brdflutGenerator.Init({512, 512});
            specularMapGenerator.Init({128, 128}, 6);

            shadowMapSampler = Graphics::CreateSampler({
                .addressMode = TextureAddressMode::ClampToEdge,
                .maxLod = 1.0,
                .borderColor = BorderColor::FloatOpaqueWhite
            });
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            const CameraData& cameraData = graph->GetCameraData();

            LightingData data{
                .viewPos = Math::MakeVec4(cameraData.viewPos, 0.0),
                .view = cameraData.view,
            };

            if (DirectionalLight* directionalLight = RenderStorage::GetDirectionalLight())
            {
                data.directionalLight[0].color = directionalLight->color.ToVec4();
                data.directionalLight[0].direction = directionalLight->direction;
                data.directionalLight[0].intensityIndirect.x = directionalLight->intensity;
                data.directionalLight[0].intensityIndirect.y = directionalLight->indirectMultipler;
                data.lightCount[0] = 1;
            }
            else
            {
                data.lightCount[0] = 0;
            }

            RenderGraphResource* skybox = node->GetInputResource("Skybox");
            if (skybox->reference != skyboxReference)
            {
                diffuseIrradianceGenerator.Generate(cmd, skybox->texture);
                specularMapGenerator.Generate(cmd, skybox->texture);
                skyboxReference = skybox->reference;
            }

            RenderGraphResource* gbufferColor = node->GetInputResource("GBufferColorMetallic");
            RenderGraphResource* gBufferNormalRoughness = node->GetInputResource("GBufferNormalRoughness");
            RenderGraphResource* gBufferPositionAO = node->GetInputResource("GBufferPositionAO");
            RenderGraphResource* shadowDepthTexture = node->GetInputResource("ShadowDepthTexture");
            RenderGraphResource* lightColor = node->GetOutputResource("LightColor");

            ShadowMapDataInfo* shadowMapDataInfo = static_cast<ShadowMapDataInfo*>(shadowDepthTexture->reference);

            for (int i = 0; i < FY_SHADOW_MAP_CASCADE_COUNT; ++i)
            {
                data.cascadeSplits[i] = shadowMapDataInfo->cascadeSplit[i];
                data.cascadeViewProjMat[i] = shadowMapDataInfo->cascadeViewProjMat[i];
            }

            bindingSet->GetVar("gbufferColorMetallic")->SetTexture(gbufferColor->texture);
            bindingSet->GetVar("gbufferNormalRoughness")->SetTexture(gBufferNormalRoughness->texture);
            bindingSet->GetVar("gBufferPositionAO")->SetTexture(gBufferPositionAO->texture);
            bindingSet->GetVar("depthTex")->SetTexture(node->GetInputTexture("Depth"));
            bindingSet->GetVar("lightColor")->SetTexture(lightColor->texture);
            bindingSet->GetVar("diffuseIrradiance")->SetTexture(diffuseIrradianceGenerator.GetTexture());
            bindingSet->GetVar("brdfLUT")->SetTexture(brdflutGenerator.GetTexture());
            bindingSet->GetVar("specularMap")->SetTexture(specularMapGenerator.GetTexture());
            bindingSet->GetVar("shadowMapTexture")->SetTexture(shadowDepthTexture->texture);
            bindingSet->GetVar("shadowMapSampler")->SetSampler(shadowMapSampler);
            bindingSet->GetVar("data")->SetValue(&data, sizeof(LightingData));

            cmd.BindPipelineState(lightingPSO);
            cmd.BindBindingSet(lightingPSO, bindingSet);

            cmd.Dispatch(std::ceil(gbufferColor->textureCreation.extent.width / 16.f),
                         std::ceil(gbufferColor->textureCreation.extent.height / 16.f),
                         1.f);
        }

        void Destroy() override
        {
            Graphics::DestroySampler(shadowMapSampler);
            Graphics::DestroyBindingSet(bindingSet);
            Graphics::DestroyComputePipelineState(lightingPSO);

            diffuseIrradianceGenerator.Destroy();
            brdflutGenerator.Destroy();
            specularMapGenerator.Destroy();

        }

        static void RegisterType(NativeTypeHandler<LightingRenderPass>& type)
        {
            RenderGraphPassBuilder<LightingRenderPass>::Builder(RenderGraphPassType::Compute)
                .Input(RenderGraphResourceCreation{
                    .name = "Skybox",
                    .type = RenderGraphResourceType::Reference,
                })
                .Input(RenderGraphResourceCreation{
                    .name = "GBufferColorMetallic",
                    .type = RenderGraphResourceType::Texture,
                    .format = Format::RGBA16F
                })
                .Input(RenderGraphResourceCreation{
                    .name = "GBufferNormalRoughness",
                    .type = RenderGraphResourceType::Texture,
                    .format = Format::RGBA16F
                })
                .Input(RenderGraphResourceCreation{
                    .name = "GBufferPositionAO",
                    .type = RenderGraphResourceType::Texture,
                    .format = Format::RGBA32F
                })
                .Input(RenderGraphResourceCreation{
                    .name = "Depth",
                    .type = RenderGraphResourceType::Texture,
                    .format = Format::Depth
                })
                .Input(RenderGraphResourceCreation{
                    .name = "ShadowDepthTexture",
                    .type = RenderGraphResourceType::Reference,
                })
                .Output(RenderGraphResourceCreation{
                    .name = "LightColor",
                    .type = RenderGraphResourceType::Texture,
                    .scale = {1.0, 1.0},
                    .format = Format::RGBA16F
                });
        }
    };

    void RegisterLightingRenderPass()
    {
        Registry::Type<LightingRenderPass>();
    }
}
