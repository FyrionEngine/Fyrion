#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/ShaderAsset.hpp"

namespace Fyrion
{
    class SceneRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        ShaderAsset* defaultShader = nullptr;

        void Init() override
        {
            defaultShader = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Test.fy_shader");
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            // cmd.Draw(3, 1, 0, 0);
        }

        static void RegisterType(NativeTypeHandler<SceneRenderPass>& type)
        {
            RenderGraphPassBuilder<SceneRenderPass>::Builder(RenderGraphPassType::Graphics)
                .Output(RenderGraphResourceCreation{
                    .name = "Color",
                    .type = RenderGraphResourceType::Attachment,
                    .scale = {1.0, 1.0},
                    .cleanValue = {100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1.0f},
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
