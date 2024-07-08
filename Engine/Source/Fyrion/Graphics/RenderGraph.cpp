#include "RenderGraph.hpp"

#include "Fyrion/Core/Graph.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/StringUtils.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::RenderGraph", LogLevel::Debug);
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

    void RenderGraphAsset::RegisterType(NativeTypeHandler<RenderGraphAsset>& type)
    {
        type.Field<&RenderGraphAsset::passes>("passes");
        type.Field<&RenderGraphAsset::edges>("edges");
    }

    Array<String> RenderGraphAsset::GetPasses() const
    {
        return passes;
    }

    Array<RenderGraphEdge> RenderGraphAsset::GetEdges() const
    {
        return edges;
    }

    RenderGraph::RenderGraph(Extent extent, RenderGraphAsset* asset): asset(asset), extent(extent)
    {
        Graph<String, SharedPtr<RenderGraphNode>> graph{};
        HashMap<String, HashMap<String, String>> edges{};

        for(const auto& pass: asset->GetPasses())
        {
            graph.AddNode(pass, MakeShared<RenderGraphNode>());
        }

        for (const auto& edge: asset->GetEdges())
        {
            graph.AddEdge(edge.nodeDest, edge.nodeOrigin);
            logger.Debug("added graph edge {} {}", edge.nodeDest, edge.nodeOrigin);

            // auto it = edges.Find(destNode);
            // if (it == edges.end())
            // {
            //     it = edges.Insert(destNode, HashMap<String, String>()).first;
            // }
            // it->second.Insert(MakePair(String{destEdge}, String{edge.origin}));
            // logger.Debug("added edge {} {} {}", destNode, destEdge, edge.origin);
        }

        auto nodes = graph.Sort();
    }

    Extent RenderGraph::GetViewportExtent() const
    {
        return extent;
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

    }
}
