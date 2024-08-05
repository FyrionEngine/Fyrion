#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/RenderStorage.hpp"
#include "Fyrion/Graphics/RenderUtils.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"

namespace Fyrion
{
    class SkyboxGenRenderPass : public RenderGraphPass
    {
    public:
        FY_BASE_TYPES(RenderGraphPass);

        EquirectangularToCubemap equirectangularToCubemap;
        TextureAsset*            currentSky{};

        void Init() override
        {
            equirectangularToCubemap.Init({512, 512}, Format::RGBA16F);

            RenderCommands& cmd = Graphics::GetCmd();
            cmd.Begin();
            equirectangularToCubemap.Convert(cmd, Graphics::GetDefaultTexture());
            cmd.SubmitAndWait(Graphics::GetMainQueue());
        }

        void Render(f64 deltaTime, RenderCommands& cmd) override
        {
            RenderGraphResource* skybox = node->GetOutputResource("Skybox");
            skybox->texture = equirectangularToCubemap.GetTexture();

            if (currentSky != RenderStorage::GetSkybox())
            {
                currentSky = RenderStorage::GetSkybox();
                if (currentSky)
                {
                    equirectangularToCubemap.Convert(cmd, currentSky->GetTexture());
                    skybox->reference = currentSky;
                }
                else
                {
                    equirectangularToCubemap.Convert(cmd, Graphics::GetDefaultTexture());
                    skybox->reference = nullptr;
                }
            }
        }

        void Destroy() override
        {
            equirectangularToCubemap.Destroy();
        }

        static void RegisterType(NativeTypeHandler<SkyboxGenRenderPass>& type)
        {
            RenderGraphPassBuilder<SkyboxGenRenderPass>::Builder(RenderGraphPassType::Compute)
                .Output(RenderGraphResourceCreation{
                    .name = "Skybox",
                    .type = RenderGraphResourceType::Reference,
                });
        }
    };


    void RegisterSkyboxGenRenderPass()
    {
        Registry::Type<SkyboxGenRenderPass>();
    }
}
