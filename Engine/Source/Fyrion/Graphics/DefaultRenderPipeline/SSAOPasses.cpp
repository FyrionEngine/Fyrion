#include <ctime>
#include <random>

#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/ImGui/ImGui.hpp"

#define SSAO_KERNEL_ARRAY_SIZE 64
#define SSAO_KERNEL_SIZE 64
#define SSAO_RADIUS 0.5
#define SSAO_NOISE_DIM 8

namespace Fyrion
{
    struct UBOSSAOKernel
    {
        Vec4 samples[SSAO_KERNEL_ARRAY_SIZE];
    };

    struct UBO
    {
        Mat4 projection;
        Mat4 view;
    };

    struct SSAOPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState;
        BindingSet*   bindingSet;
        Texture       noiseTexture;
        Sampler       colorSampler;
        Sampler       noiseSampler;

        void Init() override
        {
            ShaderAsset* shaderAsset = AssetManager::LoadByPath<ShaderAsset>("Fyrion://Shaders/Passes/SSAO.comp");

            pipelineState = Graphics::CreateComputePipelineState({
                .shader = shaderAsset
            });

            bindingSet = Graphics::CreateBindingSet(shaderAsset);

            UBOSSAOKernel kernel{};

            std::default_random_engine     rndEngine(static_cast<unsigned>(time(nullptr)));
            std::uniform_real_distribution rndDist(0.0f, 1.0f);

            for (uint32_t i = 0; i < SSAO_KERNEL_SIZE; ++i)
            {
                Vec3 sample{
                    (rndDist(rndEngine) * 2.0f - 1.f),
                    (rndDist(rndEngine) * 2.0f - 1.f),
                    (rndDist(rndEngine))
                };
                sample = Math::Normalize(sample);
                sample = sample * rndDist(rndEngine);

                float scale = float(i) / float(SSAO_KERNEL_SIZE);
                scale = std::lerp(0.1f, 1.0f, scale * scale);
                kernel.samples[i] = Vec4(sample * scale, 0.0f);
            }

            Array<Vec4> noiseValues(SSAO_NOISE_DIM * SSAO_NOISE_DIM);
            for (uint32_t i = 0; i < static_cast<uint32_t>(noiseValues.Size()); i++)
            {
                noiseValues[i] = Vec4(rndDist(rndEngine) * 2.0f - 1.0f, rndDist(rndEngine) * 2.0f - 1.0f, 0.0f, 0.0f);
            }

            noiseTexture = Graphics::CreateTexture(TextureCreation{
                .extent = {SSAO_NOISE_DIM, SSAO_NOISE_DIM},
                .format = Format::RGBA32F
            });

            TextureDataRegion region{
                .extent = {SSAO_NOISE_DIM, SSAO_NOISE_DIM},
            };

            Graphics::UpdateTextureData(TextureDataInfo{
                .texture = noiseTexture,
                .data = reinterpret_cast<const u8*>(noiseValues.begin()),
                .size = noiseValues.Size() * sizeof(Vec4),
                .regions = {&region, 1}
            });


            noiseSampler = Graphics::CreateSampler(SamplerCreation{
                .filter = SamplerFilter::Nearest,
                .addressMode = TextureAddressMode::Repeat,
            });

            colorSampler = Graphics::CreateSampler(SamplerCreation{
                .filter = SamplerFilter::Nearest,
                .addressMode = TextureAddressMode::ClampToEdge
            });

            bindingSet->GetVar("ssaoNoiseTexture")->SetTexture(noiseTexture);
            bindingSet->GetVar("colorSampler")->SetSampler(colorSampler);
            bindingSet->GetVar("ssaoNoiseSampler")->SetSampler(noiseSampler);
            bindingSet->GetVar("uboSSAOKernel")->SetValue(&kernel, sizeof(UBOSSAOKernel));
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            RenderGraphResource* gBufferNormalRoughness = node->GetInputResource("GBufferNormalRoughness");
            RenderGraphResource* gBufferPositionAO = node->GetInputResource("GBufferPositionAO");
            RenderGraphResource* ssaoTexture = node->GetOutputResource("SSAOTexture");

            bindingSet->GetVar("texturePositionDepth")->SetTexture(gBufferPositionAO->texture);
            bindingSet->GetVar("textureNormal")->SetTexture(gBufferNormalRoughness->texture);
            bindingSet->GetVar("ssaoTexture")->SetTexture(ssaoTexture->texture);


            cmd.BindPipelineState(pipelineState);
            cmd.BindBindingSet(pipelineState, bindingSet);

            UBO ubo{
                .projection = graph->GetCameraData().projection,
                .view = graph->GetCameraData().view
            };
            cmd.PushConstants(pipelineState, ShaderStage::Compute, &ubo, sizeof(UBO));

            cmd.ResourceBarrier(ResourceBarrierInfo{
                .texture = ssaoTexture->texture,
                .oldLayout = ResourceLayout::ShaderReadOnly,
                .newLayout = ResourceLayout::General
            });

            cmd.Dispatch(std::ceil(ssaoTexture->textureCreation.extent.width / 16.f),
                         std::ceil(ssaoTexture->textureCreation.extent.height / 16.f),
                         1.f);


            cmd.ResourceBarrier(ResourceBarrierInfo{
                .texture = ssaoTexture->texture,
                .oldLayout = ResourceLayout::General,
                .newLayout = ResourceLayout::ShaderReadOnly
            });


            ImGui::SetNextWindowSize(ImVec2(800, 800), ImGuiCond_Appearing);
            ImGui::Begin("debug");
            ImGui::TextureItem(ssaoTexture->texture, {(f32)ssaoTexture->textureCreation.extent.width, (f32)ssaoTexture->textureCreation.extent.height});
            ImGui::End();
        }

        void Destroy() override
        {
            Graphics::DestroyComputePipelineState(pipelineState);
            Graphics::DestroyBindingSet(bindingSet);
            Graphics::DestroyTexture(noiseTexture);
        }

        static void RegisterType(NativeTypeHandler<SSAOPass>& type)
        {
            RenderGraphPassBuilder<SSAOPass>::Builder(RenderGraphPassType::Compute)
                .Input(RenderGraphResourceCreation{
                    .name = "GBufferNormalRoughness",
                    .type = RenderGraphResourceType::Texture,
                })
                .Input(RenderGraphResourceCreation{
                    .name = "GBufferPositionAO",
                    .type = RenderGraphResourceType::Texture,
                })
                .Output(RenderGraphResourceCreation{
                    .name = "SSAOTexture",
                    .type = RenderGraphResourceType::Texture,
                    .scale = {0.5, 0.5},
                    .format = Format::R
                });
        }
    };

    struct SSAOBlurPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        void Init() override {}

        void Render(f64 deltaTime, RenderCommands& cmd) override {}

        void Destroy() override {}
    };


    void RegisterSSAOPasses()
    {
        // Registry::Type<SSAOPass>();
        // Registry::Type<SSAOBlurPass>();
    }
}
