#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/RenderPipeline.hpp"
#include "Fyrion/Graphics/Assets/MeshAsset.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"

namespace Fyrion
{
    class DefaultRenderPipeline : public RenderPipeline
    {
    private:
        PipelineState pipelineState{};
        BindingSet*   bindingSet{};
    public:

        FY_BASE_TYPES(RenderPipeline);

        void BuildRenderGraph(RenderGraph& rg) override
        {
            Array<MeshRenderData> meshes;

            RenderGraphResource* depth = rg.Create(RenderGraphResourceCreation{
                .name = "depth",
                .type = RenderGraphResourceType::Attachment,
                .scale = {1, 1},
                .format = Format::Depth,
            });

            RenderGraphResource* shadow = rg.Create(RenderGraphResourceCreation{
                .name = "shadow",
                .type = RenderGraphResourceType::Attachment,
                .size = {4096, 4096, 1},
                .format = Format::Depth,
            });

            RenderGraphResource* color = rg.Create(RenderGraphResourceCreation{
                .name = "color",
                .type = RenderGraphResourceType::Attachment,
                .scale = {1, 1},
                .format = Format::RGBA
            });

            RenderGraphResource* light = rg.Create(RenderGraphResourceCreation{
                .name = "light",
                .type = RenderGraphResourceType::Attachment,
                .scale = {1, 1},
                .format = Format::RGBA16F
            });

            RenderGraphResource* final = rg.Create(RenderGraphResourceCreation{
                .name = "final",
                .type = RenderGraphResourceType::Attachment,
                .scale = {1, 1},
                .format = Format::RGBA16F
            });

            rg.AddPass("GBuffer", RenderGraphPassType::Graphics)
              .Write(color)
              .Write(depth)
              .ClearColor(Vec4{0, 0, 0, 1})
              .ClearDepth(true)
              .Init([&](RenderGraphPass& node)
              {
                  GraphicsPipelineCreation graphicsPipelineCreation{
                      .shader = Assets::LoadByPath<ShaderAsset>("Fyrion://Shaders/Passes/GBufferRender.raster"),
                      .renderPass = node.GetRenderPass(),
                      .depthWrite = true,
                      .cullMode = CullMode::Back,
                      .compareOperator = CompareOp::Less,
                  };

                  pipelineState = Graphics::CreateGraphicsPipelineState(graphicsPipelineCreation);
                  bindingSet = Graphics::CreateBindingSet(graphicsPipelineCreation.shader);
              })
              .Render([&](RenderGraphPass& node, RenderCommands& cmd)
              {
                  cmd.BindPipelineState(pipelineState);
                  cmd.BindBindingSet(pipelineState, bindingSet);

                  // for (MeshRenderData& meshRenderData : meshes)
                  // {
                  //     if (MeshAsset* mesh = meshRenderData.mesh)
                  //     {
                  //         Span<MeshPrimitive> primitives = mesh->GetPrimitives();
                  //
                  //         cmd.BindVertexBuffer(mesh->GetVertexBuffer());
                  //         cmd.BindIndexBuffer(mesh->GetIndexBuffeer());
                  //
                  //         cmd.PushConstants(pipelineState, ShaderStage::Vertex, &meshRenderData.model, sizeof(Mat4));
                  //
                  //         for (MeshPrimitive& primitive : primitives)
                  //         {
                  //             if (MaterialAsset* material = meshRenderData.materials[primitive.materialIndex])
                  //             {
                  //                 cmd.BindBindingSet(pipelineState, material->GetBindingSet());
                  //                 cmd.DrawIndexed(primitive.indexCount, 1, primitive.firstIndex, 0, 0);
                  //             }
                  //         }
                  //     }
                  // }
              });


            rg.AddPass("light", RenderGraphPassType::Compute)
              .Read(color)
              .Read(depth)
              .Read(shadow)
              .Write(light);

            rg.AddPass("shadows", RenderGraphPassType::Graphics)
              .Write(shadow)
              .ClearDepth(true);

            rg.AddPass("AO", RenderGraphPassType::Compute)
              .Write(final)
              .Read(depth)
              .Read(light);

        }
    };


    void RegisterDefaultRenderPipeline()
    {
        Registry::Type<DefaultRenderPipeline>();
    }
}
