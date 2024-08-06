#include "DefaultRenderPipelineTypes.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/RenderStorage.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/Graphics/Assets/MeshAsset.hpp"


namespace Fyrion
{
    struct ShadowMapRenderPass : RenderGraphPass
    {
        FY_BASE_TYPES(RenderGraphPass);

        float cascadeSplitLambda = 0.95f;

        Texture       shadowMapTexture{};
        PipelineState pipelineState{};

        TextureView shadowMapTextureViews[FY_SHADOW_MAP_CASCADE_COUNT];
        RenderPass  shadowMapPass[FY_SHADOW_MAP_CASCADE_COUNT];

        ShadowMapDataInfo shadowMapDataInfo{};

        struct PushConsts
        {
            Mat4 model;
            Mat4 viewProjection;
        };

        void Init() override
        {
            shadowMapTexture = Graphics::CreateTexture(TextureCreation{
                .extent = {FY_SHADOW_MAP_DIM, FY_SHADOW_MAP_DIM},
                .format = Format::Depth,
                .usage = TextureUsage::DepthStencil | TextureUsage::ShaderResource,
                .arrayLayers = FY_SHADOW_MAP_CASCADE_COUNT
            });

            Graphics::UpdateTextureLayout(shadowMapTexture, ResourceLayout::Undefined, ResourceLayout::DepthStencilReadOnly, true);

            for (u32 i = 0; i < FY_SHADOW_MAP_CASCADE_COUNT; ++i)
            {
                shadowMapTextureViews[i] = Graphics::CreateTextureView(TextureViewCreation{
                    .texture = shadowMapTexture,
                    .baseArrayLayer = i,
                });

                AttachmentCreation attachmentCreation{
                    .textureView = shadowMapTextureViews[i],
                    .finalLayout = ResourceLayout::DepthStencilAttachment,
                };
                shadowMapPass[i] = Graphics::CreateRenderPass(RenderPassCreation{
                    .attachments = {&attachmentCreation, 1}
                });
            }

            GraphicsPipelineCreation graphicsPipelineCreation{
                .shader = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Passes/ShadowMap.raster"),
                .renderPass = shadowMapPass[0],
                .depthWrite = true,
                .cullMode = CullMode::Front,
                .compareOperator = CompareOp::LessOrEqual,
                .stride = sizeof(VertexStride)
            };

            pipelineState = Graphics::CreateGraphicsPipelineState(graphicsPipelineCreation);
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            RenderGraphResource* shadowDepthTexture = node->GetOutputResource("ShadowDepthTexture");
            shadowDepthTexture->texture = shadowMapTexture;
            shadowDepthTexture->reference = &shadowMapDataInfo;

            if (DirectionalLight* directionalLight = RenderStorage::GetDirectionalLight(); directionalLight != nullptr && directionalLight->castShadows)
            {
                const CameraData& cameraData = graph->GetCameraData();

                float nearClip = cameraData.nearClip;
                float farClip = cameraData.farClip;
                float clipRange = farClip - nearClip;

                float minZ = nearClip;
                float maxZ = nearClip + clipRange;

                float range = maxZ - minZ;
                float ratio = maxZ / minZ;

                // Calculate split depths based on view camera frustum
                // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
                for (uint32_t i = 0; i < FY_SHADOW_MAP_CASCADE_COUNT; i++)
                {
                    float p = (i + 1) / static_cast<float>(FY_SHADOW_MAP_CASCADE_COUNT);
                    float log = minZ * std::pow(ratio, p);
                    float uniform = minZ + range * p;
                    float d = cascadeSplitLambda * (log - uniform) + uniform;
                    shadowMapDataInfo.cascadeSplits[i] = (d - nearClip) / clipRange;
                }

                // Calculate orthographic projection matrix for each cascade
                float lastSplitDist = 0.0;
                for (uint32_t i = 0; i < FY_SHADOW_MAP_CASCADE_COUNT; i++)
                {
                    float splitDist = shadowMapDataInfo.cascadeSplits[i];

                    Vec3 frustumCorners[8] = {
                        Vec3{-1.0f, 1.0f, 0.0f},
                        Vec3{1.0f, 1.0f, 0.0f},
                        Vec3{1.0f, -1.0f, 0.0f},
                        Vec3{-1.0f, -1.0f, 0.0f},
                        Vec3{-1.0f, 1.0f, 1.0f},
                        Vec3{1.0f, 1.0f, 1.0f},
                        Vec3{1.0f, -1.0f, 1.0f},
                        Vec3{-1.0f, -1.0f, 1.0f},
                    };

                    // Project frustum corners into world space
                    Mat4 invCam = Math::Inverse(cameraData.projection * cameraData.view);
                    for (uint32_t j = 0; j < 8; j++)
                    {
                        Vec4 invCorner = invCam * Vec4(frustumCorners[j], 1.0f);
                        frustumCorners[j] = Math::MakeVec3(invCorner / invCorner.w);
                    }

                    for (uint32_t j = 0; j < 4; j++)
                    {
                        Vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
                        frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
                        frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
                    }

                    // Get frustum center
                    Vec3 frustumCenter = Vec3();
                    for (uint32_t j = 0; j < 8; j++)
                    {
                        frustumCenter += frustumCorners[j];
                    }
                    frustumCenter /= 8.0f;

                    float radius = 0.0f;
                    for (uint32_t j = 0; j < 8; j++)
                    {
                        float distance = Math::Len(frustumCorners[j] - frustumCenter);
                        radius = Math::Max(radius, distance);
                    }
                    radius = std::ceil(radius * 16.0f) / 16.0f;

                    Vec3 maxExtents = Vec3{radius, radius, radius};
                    Vec3 minExtents = -maxExtents;

                    Vec3 lightDir = Math::Normalize(-Math::MakeVec3(directionalLight->direction));
                    Mat4 lightViewMatrix = Math::LookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, Vec3{0.0f, 1.0f, 0.0f});
                    Mat4 lightOrthoMatrix = Math::Ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

                    // Store split distance and matrix in cascade
                    f32 splitDepth = (cameraData.nearClip + splitDist * clipRange) * -1.0f;
                    shadowMapDataInfo.cascadeViewProjMat[i] = lightOrthoMatrix * lightViewMatrix;

                    lastSplitDist = shadowMapDataInfo.cascadeSplits[i];

                    ClearDepthStencilValue depthStencilValue{
                    };

                    //render
                    cmd.BeginRenderPass(BeginRenderPassInfo{
                        .renderPass = shadowMapPass[i],
                        .depthStencil = &depthStencilValue
                    });

                    cmd.SetViewport(ViewportInfo{
                        .x = 0.0f,
                        .y = 0.0f,
                        .width = FY_SHADOW_MAP_DIM,
                        .height = FY_SHADOW_MAP_DIM,
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f,
                    });

                    cmd.SetScissor(Rect{0, 0, FY_SHADOW_MAP_DIM, FY_SHADOW_MAP_DIM});
                    cmd.BindPipelineState(pipelineState);

                    for (MeshRenderData& meshRenderData : RenderStorage::GetMeshesToRender())
                    {
                        if (MeshAsset* mesh = meshRenderData.mesh)
                        {
                            Span<MeshPrimitive> primitives = mesh->GetPrimitives();

                            cmd.BindVertexBuffer(mesh->GetVertexBuffer());
                            cmd.BindIndexBuffer(mesh->GetIndexBuffeer());

                            PushConsts pushConsts{
                                .model = meshRenderData.model,
                                .viewProjection = shadowMapDataInfo.cascadeViewProjMat[i],
                            };

                            cmd.PushConstants(pipelineState, ShaderStage::Vertex, &pushConsts, sizeof(PushConsts));

                            for (MeshPrimitive& primitive : primitives)
                            {
                                if (MaterialAsset* material = meshRenderData.materials[primitive.materialIndex])
                                {
                                    cmd.DrawIndexed(primitive.indexCount, 1, primitive.firstIndex, 0, 0);
                                }
                            }
                        }
                    }
                    cmd.EndRenderPass();

                    cmd.ResourceBarrier(ResourceBarrierInfo{
                        .texture = shadowMapTexture,
                        .oldLayout = ResourceLayout::DepthStencilAttachment,
                        .newLayout = ResourceLayout::DepthStencilReadOnly,
                        .baseArrayLayer = i,
                        .isDepth = true
                    });
                }
            }
        }

        void Destroy() override
        {
            for (u32 i = 0; i < FY_SHADOW_MAP_CASCADE_COUNT; ++i)
            {
                Graphics::DestroyRenderPass(shadowMapPass[i]);
                Graphics::DestroyTextureView(shadowMapTextureViews[i]);
            }

            Graphics::DestroyTexture(shadowMapTexture);
            Graphics::DestroyGraphicsPipelineState(pipelineState);
        }

        static void RegisterType(NativeTypeHandler<ShadowMapRenderPass>& type)
        {
            RenderGraphPassBuilder<ShadowMapRenderPass>::Builder(RenderGraphPassType::Other)
                .Output(RenderGraphResourceCreation{
                    .name = "ShadowDepthTexture",
                    .type = RenderGraphResourceType::Reference,
                });
        }
    };


    void RegisterShadowMapRenderPass()
    {
        Registry::Type<ShadowMapRenderPass>();
    }
}
