#include "RenderGraph.hpp"

#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::RenderGraph");
        bool registerSwapchainRenderEvent = true;
        HashMap<String, RenderGraphPassCreation> passDatabase{};
    }

    void RenderGraph::SetRegisterSwapchainRenderEvent(bool p_registerSwapchainRenderEvent)
    {
        registerSwapchainRenderEvent = p_registerSwapchainRenderEvent;
    }

    void RenderGraph::RegisterPass(const RenderGraphPassCreation& renderGraphPassCreation)
    {
        logger.Debug("pass {} Registered", renderGraphPassCreation.name);
        passDatabase.Emplace(renderGraphPassCreation.name, RenderGraphPassCreation{renderGraphPassCreation});
    }

    Extent RenderGraph::GetViewportExtent() const
    {
        return extent;
    }

    bool RenderGraph::IsLoaded() const
    {
        return loaded;
    }

    void RenderGraph::Load(Extent p_extent)
    {
        extent = p_extent;
        loaded = true;
    }

    void RenderGraph::Resize(Extent p_extent)
    {
        extent = p_extent;
    }

    Texture RenderGraph::GetColorOutput() const
    {
        return {};
    }

    Texture RenderGraph::GetDepthOutput() const
    {
        return {};
    }

    void RenderGraph::RegisterType(NativeTypeHandler<RenderGraph>& type)
    {
        type.Field<&RenderGraph::passes>("passes");
        type.Field<&RenderGraph::edges>("edges");
    }
}
