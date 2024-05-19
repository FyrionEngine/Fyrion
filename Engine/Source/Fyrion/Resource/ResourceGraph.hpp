#pragma once
#include "ResourceTypes.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Allocator.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{
    class TypeHandler;
    class FunctionHandler;

    struct ResourceGraphOutput
    {
        String label{};
    };

    struct ResourceGraphNode
    {
        String label{};
    };

    class ResourceGraph;

    struct GraphInstanceAsset
    {
        constexpr static u32 Graph = 0;
        constexpr static u32 Inputs = 0;
    };

    struct ResourceGraphAsset
    {
        constexpr static u32 Nodes = 0;
        constexpr static u32 Links = 1;
    };

    struct ResourceGraphNodeValue
    {
        String       name{};
        TypeHandler* typeHandler{};
        ConstPtr     value{};
        bool         publicValue{};
    };

    struct ResourceGraphNodeInfo
    {
        u32                           id{};
        FunctionHandler*              functionHandler{};
        TypeHandler*                  typeHandler{};
        Array<ResourceGraphNodeValue> values{};
    };

    struct ResourceGraphLinkInfo
    {
        u32    outputNodeId{};
        String outputPin{};
        u32    inputNodeId{};
        String inputPin{};
    };

    struct ResourceGraphNodeParamData
    {
        u32          offset = U32_MAX;
        TypeHandler* typeHandler{};
        VoidPtr      defaultValue{};
        u32          copyOffset = U32_MAX;
        bool         offsetFromInstance = false;
    };

    class FY_API ResourceGraphNodeData
    {
    public:
        ResourceGraphNodeData(ResourceGraph* resourceGraph, const ResourceGraphNodeInfo& info);

        FY_FINLINE u32              GetId() const { return m_id; }
        FY_FINLINE TypeHandler*     GetTypeHandler() const { return m_typeHandler; }
        FY_FINLINE FunctionHandler* GetFunctionHandler() const { return m_functionHandler; }

        friend class ResourceGraphInstance;
        friend class ResourceGraph;

    private:
        ResourceGraph*                         m_resourceGraph{};
        ResourceGraphNodeInfo                  m_info{};
        u32                                    m_id{};
        bool                                   m_valid = true;
        FunctionHandler*                       m_functionHandler{};
        TypeHandler*                           m_typeHandler{};
        u32                                    m_outputOffset = U32_MAX;
        HashMap<String, u32>                   m_offsets{};
        HashMap<String, ResourceGraphLinkInfo> m_inputLinks{};
    };

    class FY_API ResourceGraphInstance
    {
    public:
        ResourceGraphInstance(ResourceGraph* resourceGraph);

        void           SetInputValue(const StringView& inputName, ConstPtr data);
        Span<ConstPtr> GetOutputs(TypeID typeId) const;
        void           Destroy();

        void           Execute();

        template <typename T>
        const T* GetOutput(u32 index) const
        {
            Span<ConstPtr> outputs = GetOutputs(GetTypeID<T>());
            return static_cast<const T*>(outputs[index]);
        }

    private:
        ResourceGraph*                   m_resourceGraph;
        CharPtr                          m_instanceData;
        Array<Array<VoidPtr>>            m_nodeParams;
        HashMap<TypeID, Array<ConstPtr>> m_outputs;
    };


    class FY_API ResourceGraph
    {
    public:

        void SetGraph(const Span<ResourceGraphNodeInfo>& nodes,
                      const Span<ResourceGraphLinkInfo>& links);

        void SetGraph(RID assetGraph);

        ResourceGraphInstance*                 CreateInstance();
        Span<SharedPtr<ResourceGraphNodeData>> GetNodes() const;

        void Destroy();

        friend class ResourceGraphInstance;
        friend class ResourceGraphNodeData;

    private:
        Allocator& m_allocator = MemoryGlobals::GetDefaultAllocator();

        Array<SharedPtr<ResourceGraphNodeData>> m_nodes;
        Array<ResourceGraphLinkInfo>            m_links;
        Array<ResourceGraphNodeParamData>       m_data;
        HashMap<String, u32>                    m_publicInputs;
        HashMap<TypeID, Array<u32>>             m_outputs;

        u32 m_instanceAllocRequiredSize = 0;
    };
}
