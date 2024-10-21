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
        void BuildRenderGraph(RenderGraph& rg) override
        {
            Array<MeshRenderData> meshes;

            RenderGraphResource* depth = rg.Create(RenderGraphResourceCreation{
                .name = "depth",
                .type = RenderGraphResourceType::Texture,
                .scale = {1, 1},
                .loadOp = LoadOp::Load,
                .format = Format::Depth,
            });

            RenderGraphResource* color = rg.Create(RenderGraphResourceCreation{
                .name = "color",
                .type = RenderGraphResourceType::Texture,
                .scale = {1, 1},
                .format = Format::RGBA
            });

            rg.AddPass("GBuffer", RenderGraphPassType::Graphics)
              .Write(color)
              .Write(depth)
              .Init([&](RenderGraphNode& node)
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
              .Render([&](RenderGraphNode& node, RenderCommands& cmd)
              {
                  cmd.BindPipelineState(pipelineState);
                  cmd.BindBindingSet(pipelineState, bindingSet);

                  for (MeshRenderData& meshRenderData : meshes)
                  {
                      if (MeshAsset* mesh = meshRenderData.mesh)
                      {
                          Span<MeshPrimitive> primitives = mesh->GetPrimitives();

                          cmd.BindVertexBuffer(mesh->GetVertexBuffer());
                          cmd.BindIndexBuffer(mesh->GetIndexBuffeer());

                          cmd.PushConstants(pipelineState, ShaderStage::Vertex, &meshRenderData.model, sizeof(Mat4));

                          for (MeshPrimitive& primitive : primitives)
                          {
                              if (MaterialAsset* material = meshRenderData.materials[primitive.materialIndex])
                              {
                                  cmd.BindBindingSet(pipelineState, material->GetBindingSet());
                                  cmd.DrawIndexed(primitive.indexCount, 1, primitive.firstIndex, 0, 0);
                              }
                          }
                      }
                  }
              });
        }
    };
}
