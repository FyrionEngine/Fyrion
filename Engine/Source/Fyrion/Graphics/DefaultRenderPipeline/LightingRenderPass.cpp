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
        Mat4                 viewInverse{};
        Mat4                 projInverse{};
        Vec4                 skyColor{};
        Vec4                 viewPos{};
        DirectionalLightData directionalLight[4];
        alignas(16) u32      directionalLightCount = 0;
    };

    class LightingRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState lightingPSO{};
        BindingSet*   bindingSet{};

        TextureAsset* skyTexture{};

        DiffuseIrradianceGenerator diffuseIrradianceGenerator;
        EquirectangularToCubemap   equirectangularToCubemap;
        BRDFLUTGenerator           brdflutGenerator;
        SpecularMapGenerator       specularMapGenerator;

        void Init() override
        {
            ComputePipelineCreation creation{
                .shader = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Lighting.comp")
            };

            lightingPSO = Graphics::CreateComputePipelineState(creation);
            bindingSet = Graphics::CreateBindingSet(creation.shader);

            equirectangularToCubemap.Init({512, 512}, Format::RGBA16F);
            diffuseIrradianceGenerator.Init({64, 64});
            brdflutGenerator.Init({512, 512});
            specularMapGenerator.Init({128, 128}, 5);
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            if (skyTexture != RenderStorage::GetSkybox())
            {
                skyTexture = RenderStorage::GetSkybox();
                if (skyTexture)
                {
                    equirectangularToCubemap.Convert(cmd, skyTexture->GetTexture());
                    diffuseIrradianceGenerator.Generate(cmd, equirectangularToCubemap.GetTexture());
                    specularMapGenerator.Generate(cmd, equirectangularToCubemap.GetTexture());
                }
            }

            const CameraData& cameraData = graph->GetCameraData();

            LightingData data{
                .viewInverse = cameraData.viewInverse,
                .projInverse = cameraData.projectionInverse,
                .skyColor = RenderStorage::GetSkybox() != nullptr ? Color::WHITE.ToVec4() : Color::BLACK.ToVec4(),
                .viewPos = Math::MakeVec4(cameraData.viewPos, 0.0),
            };

            if (DirectionalLight* directionalLight = RenderStorage::GetDirectionalLight())
            {
                data.directionalLight[0].color = directionalLight->color.ToVec4();
                data.directionalLight[0].direction = directionalLight->direction;
                data.directionalLight[0].intensityIndirect.x = directionalLight->intensity;
                data.directionalLight[0].intensityIndirect.y = directionalLight->indirectMultipler;
                data.directionalLightCount = 1;
            }
            else
            {
                data.directionalLightCount = 0;
            }

            RenderGraphResource* gbufferColor = node->GetInputResource("GBufferColorMetallic");
            RenderGraphResource* gBufferNormalRoughness = node->GetInputResource("GBufferNormalRoughness");
            RenderGraphResource* gBufferPositionAO = node->GetInputResource("GBufferPositionAO");
            RenderGraphResource* lightColor = node->GetOutputResource("LightColor");

            bindingSet->GetVar("gbufferColorMetallic")->SetTexture(gbufferColor->texture);
            bindingSet->GetVar("gbufferNormalRoughness")->SetTexture(gBufferNormalRoughness->texture);
            bindingSet->GetVar("gBufferPositionAO")->SetTexture(gBufferPositionAO->texture);
            bindingSet->GetVar("depthTex")->SetTexture(node->GetInputTexture("Depth"));
            bindingSet->GetVar("lightColor")->SetTexture(lightColor->texture);
            bindingSet->GetVar("sky")->SetTexture(RenderStorage::GetSkybox() != nullptr ? RenderStorage::GetSkybox()->GetTexture() : Texture{});
            bindingSet->GetVar("diffuseIrradiance")->SetTexture(diffuseIrradianceGenerator.GetTexture());
            bindingSet->GetVar("brdfLUT")->SetTexture(brdflutGenerator.GetTexture());
            bindingSet->GetVar("specularMap")->SetTexture(specularMapGenerator.GetTexture());
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

            diffuseIrradianceGenerator.Destroy();
            equirectangularToCubemap.Destroy();
            brdflutGenerator.Destroy();
            specularMapGenerator.Destroy();
        }

        static void RegisterType(NativeTypeHandler<LightingRenderPass>& type)
        {
            RenderGraphPassBuilder<LightingRenderPass>::Builder(RenderGraphPassType::Compute)
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
