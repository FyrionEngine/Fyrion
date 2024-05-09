#include "ResourceGraph.hpp"

namespace Fyrion
{
    ResourceGraphInstance::ResourceGraphInstance(ResourceGraph* resourceGraph) : m_resourceGraph(resourceGraph)
    {
    }

    void ResourceGraphInstance::Execute()
    {
    }

    void ResourceGraphInstance::SetInputValue(const StringView& inputName, ConstPtr data, usize size)
    {
    }

    Span<ConstPtr> ResourceGraphInstance::GetOutputs(TypeID typeId) const
    {
        return {};
    }

    void ResourceGraphInstance::Destroy()
    {
    }

    ResourceGraph::ResourceGraph(const Span<ResourceGraphInputNodeInfo>  inputs,
                                 const Span<ResourceGraphOutputNodeInfo> outputs,
                                 const Span<ResourceGraphNodeInfo>&      nodes,
                                 const Span<ResourceGraphLinkInfo>&      links)
    {
    }

    ResourceGraphInstance* ResourceGraph::CreateInstance()
    {
        return m_allocator.Alloc<ResourceGraphInstance>(this);
    }

    void RegisterResourceGraphTypes()
    {
    }
}
