#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"

namespace Fyrion
{
    class SceneRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        void Render(f64 deltaTime, const RenderCommands& cmd) override {}

        static void RegisterType(NativeTypeHandler<SceneRenderPass>& type)
        {
            RenderGraphPassBuilder<SceneRenderPass>::Builder()
                .Output(RenderGraphResourceCreation{
                    .name = "Color",
                    .type = RenderGraphResourceType::Attachment,
                    .scale = {1.0, 1.0},
                    .cleanValue = {0.0, 0.0, 0.0, 1.0}, //Color::CORNFLOWER_BLUE.ToVector4()
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
