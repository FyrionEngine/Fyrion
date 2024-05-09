#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Allocator.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{

    class TypeHandler;
    class FunctionHandler;

    struct ResourceGraphOutput;
    struct ResourceGraphNode;
    class ResourceGraph;

    struct ResourceGraphAsset
    {
        constexpr static u32 Nodes = 0;
        constexpr static u32 Links = 1;
    };

    struct ResourceGraphNodeValue
    {
        String       name;
        TypeHandler* typeHandler;
        ConstPtr     value;
    };

    struct ResourceGraphInputNodeInfo
    {
        u32          index;
        String       name;
        TypeHandler* typeHandler;
        ConstPtr     value;
    };

    struct ResourceGraphOutputNodeInfo
    {
        u32          index;
        TypeHandler* typeHandler;
    };


    struct ResourceGraphNodeInfo
    {
        u32                           index;
        FunctionHandler*              functionHandler;
        Array<ResourceGraphNodeValue> defaultValues;
    };

    struct ResourceGraphLinkInfo
    {
        u32    inputNodeIndex;
        String inputPin;
        u32    outputNodeIndex;
        String outputPin;
    };


    class FY_API ResourceGraphInstance
    {
    public:
        ResourceGraphInstance(ResourceGraph* resourceGraph);

        void           SetInputValue(const StringView& inputName, ConstPtr data, usize size);
        Span<ConstPtr> GetOutputs(TypeID typeId) const;
        void           Destroy();
        void           Execute();

        template <typename T>
        Span<T> GetOutputs() const
        {
            Span<ConstPtr> outputs = GetOutputs(GetTypeID<T>());
            return Span<T>{*static_cast<const T*>(*outputs.begin()), *static_cast<const T*>(*outputs.end())};
        }

    private:
        ResourceGraph* m_resourceGraph;
    };


    class FY_API ResourceGraph
    {
    public:
        ResourceGraph(const Span<ResourceGraphInputNodeInfo>  inputs,
                      const Span<ResourceGraphOutputNodeInfo> outputs,
                      const Span<ResourceGraphNodeInfo>&      nodes,
                      const Span<ResourceGraphLinkInfo>&      links);

        ResourceGraphInstance* CreateInstance();

    private:
        Allocator& m_allocator = MemoryGlobals::GetDefaultAllocator();
    };
}
