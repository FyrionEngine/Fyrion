#include "Fyrion/Graphics/RenderGraph.hpp"

namespace Fyrion
{
    class SkyboxRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        void Init() override
        {

        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {

        }

        void Destroy() override {}

        static void RegisterType(NativeTypeHandler<SkyboxRenderPass>& type)
        {
            RenderGraphPassBuilder<SkyboxRenderPass>::Builder(RenderGraphPassType::Compute)
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

    void RegisterSkyboxRenderPass()
    {
        Registry::Type<SkyboxRenderPass>();
    }
}
