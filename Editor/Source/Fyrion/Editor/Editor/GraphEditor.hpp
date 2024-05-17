#pragma once
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    class AssetTree;
    class TypeHandler;
    class FunctionHandler;


    enum class GraphEditorPinKind
    {
        Input,
        Output,
    };

    struct GraphEditorNodePin
    {
        RID                node;
        u64                pin;
        String             label;
        String             name;
        TypeID             typeId;
        GraphEditorPinKind kind;
    };

    struct GraphEditorNodePinLookup
    {
        RID                node;
        String             name;

        friend bool operator==(const GraphEditorNodePinLookup& lhs, const GraphEditorNodePinLookup& rhs)
        {
            return lhs.node == rhs.node && lhs.name == rhs.name;
        }
    };

    template<>
    struct Hash<GraphEditorNodePinLookup>
    {
        constexpr static bool hasHash = true;

        constexpr static usize Value(const GraphEditorNodePinLookup& lookup)
        {
            usize hash = 0;
            HashCombine(hash, HashValue(lookup.node), HashValue(lookup.name));
            return hash;
        }
    };

    struct GraphEditorNode
    {
        RID              rid;
        TypeHandler*     typeHandler;
        FunctionHandler* functionHandler;
        Vec2             position;
        String           label;
        bool             initialized;

        Array<u32> inputs;
        Array<u32> outputs;
    };

    struct GraphEditorLink
    {
        u64    linkId{};
        u64    inputPin{};
        u64    outputPin{};
        TypeID linkType{};
    };

    class GraphEditor
    {
    public:
        explicit GraphEditor(AssetTree& assetTree) : m_assetTree(assetTree)
        {
        }

        FY_NO_COPY_CONSTRUCTOR(GraphEditor)

        void                      OpenGraph(RID rid);
        bool                      IsGraphLoaded() const;
        Span<GraphEditorNode*>    GetNodes() { return m_nodesArray; }
        Span<GraphEditorNodePin*> GetPins() { return m_pinsArray; }
        Span<GraphEditorLink>     GetLinks() { return m_links; }

        void AddOutput(TypeHandler* outputType, const Vec2& position);
        void AddNode(FunctionHandler* functionHandler, const Vec2& position);
        bool ValidateLink(u64 inputPin, u64 outputPin);
        void AddLink(u64 inputPin, u64 outputPin);

    private:
        AssetTree& m_assetTree;
        RID        m_asset{};
        RID        m_graph{};
        TypeID     m_graphTypeId{};
        String     m_graphName{};

        Array<GraphEditorNode*>                  m_nodesArray;
        HashMap<RID, UniquePtr<GraphEditorNode>> m_nodes;

        Array<GraphEditorNodePin*> m_pinsArray;
        HashMap<GraphEditorNodePinLookup, UniquePtr<GraphEditorNodePin>> m_pins;

        Array<GraphEditorLink> m_links;

        void AddNodeCache(RID node);
        void AddLinkCache(RID link);

        GraphEditorNodePin* GetPin(u64 left, u64 right, GraphEditorPinKind disiredKind);

        GraphEditorNodePin* FindPin(RID node, const StringView& pin, GraphEditorPinKind graphEditorPinKind);
    };
}
