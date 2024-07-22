#include <Fyrion/Core/Color.hpp>

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/DCCAsset.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"


namespace Fyrion
{
    class TextureAsset;

    struct SceneData
    {
        Mat4 viewProjection;
        // Mat4 lightSpace;
        // Vec4 viewPos;
    };

    class SceneRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState{};
        BindingSet*   bindingSet{};
        MeshAsset*    mesh;

        void Init() override
        {
            GraphicsPipelineCreation graphicsPipelineCreation{
                .shader = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/BasicRenderer.raster"),
                .renderPass = node->GetRenderPass(),
                .depthWrite = true,
                .cullMode = CullMode::Back,
                .compareOperator = CompareOp::Greater,
            };

            pipelineState = Graphics::CreateGraphicsPipelineState(graphicsPipelineCreation);
            bindingSet = Graphics::CreateBindingSet(graphicsPipelineCreation.shader);

            TextureAsset* texture = AssetDatabase::FindByPath<TextureAsset>("NewAssetRefactor://planks-albedo.png");
            bindingSet->GetVar("texture")->SetTexture(texture->GetTexture());

            mesh = AssetDatabase::FindByPath<MeshAsset>("NewAssetRefactor://Cube/Cube.gltf#Cube");
        }


        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            const CameraData& cameraData = graph->GetCameraData();
            SceneData         data{.viewProjection = cameraData.projection * cameraData.view};
            bindingSet->GetVar("scene")->Set(data);

            cmd.BindPipelineState(pipelineState);
            cmd.BindBindingSet(pipelineState, bindingSet);

            cmd.BindVertexBuffer(mesh->GetVertexBuffer());
            cmd.BindIndexBuffer(mesh->GetIndexBuffeer());

            Mat4 model{1.0};
            cmd.PushConstants(pipelineState, ShaderStage::Vertex, &model, sizeof(Mat4));

            for (MeshPrimitive& primitive : mesh->GetPrimitives())
            {
                cmd.DrawIndexed(primitive.indexCount, 1, primitive.firstIndex, 0, 0);
            }
        }

        void Destroy() override
        {
            Graphics::DestroyGraphicsPipelineState(pipelineState);
            Graphics::DestroyBindingSet(bindingSet);
        }

        static void RegisterType(NativeTypeHandler<SceneRenderPass>& type)
        {
            RenderGraphPassBuilder<SceneRenderPass>::Builder(RenderGraphPassType::Graphics)
                .Output(RenderGraphResourceCreation{
                    .name = "Color",
                    .type = RenderGraphResourceType::Attachment,
                    .scale = {1.0, 1.0},
                    .cleanValue = Color::CORNFLOWER_BLUE.ToVec4(),
                    .format = Format::RGBA
                })
                .Output(RenderGraphResourceCreation{
                    .name = "Depth",
                    .type = RenderGraphResourceType::Attachment,
                    .scale = {1.0, 1.0},
                    .cleanDepth = true,
                    .format = Format::Depth
                });
        }
    };

    void RegisterSceneRenderPass()
    {
        Registry::Type<SceneRenderPass>();
    }
}
