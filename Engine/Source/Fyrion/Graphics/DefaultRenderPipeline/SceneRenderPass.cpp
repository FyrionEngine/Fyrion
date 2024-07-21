#include <Fyrion/Core/Color.hpp>

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"


namespace Fyrion
{
    class SceneRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        PipelineState pipelineState{};
        BindingSet* bindingSet{};

        void Init() override
        {
            GraphicsPipelineCreation graphicsPipelineCreation{
                .shader = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/BasicRenderer.raster"),
                .renderPass = node->GetRenderPass()
            };

            pipelineState = Graphics::CreateGraphicsPipelineState(graphicsPipelineCreation);
            bindingSet = Graphics::CreateBindingSet(graphicsPipelineCreation.shader);
        }

        void Destroy() override
        {
            Graphics::DestroyGraphicsPipelineState(pipelineState);
            Graphics::DestroyBindingSet(bindingSet);
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            // cmd.BindPipelineState(pipelineState);
            // cmd.BindBindingSet(pipelineState, bindingSet);
            // cmd.Draw(3, 1, 0, 0);
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
