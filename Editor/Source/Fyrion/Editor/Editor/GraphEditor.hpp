#pragma once
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    class AssetTree;
    class TypeHandler;
    class FunctionHandler;
    struct GraphEditorLink;
    struct GraphEditorNode;

    enum class GraphEditorPinKind
    {
        Input,
        Output,
        AddOutputPin,
        AddInputPin
    };

    struct GraphEditorNodePin
    {
        GraphEditorNode*   node;
        String             label;
        String             name;
        TypeID             typeId;
        RID                value;
        RID                valueAsset;
        GraphEditorPinKind kind;
        HashSet<RID>       links{};
        bool               publicValue = false;
        TypeID             resourceType;
    };

    struct GraphEditorNodePinLookup
    {
        RID    node;
        String name;

        friend bool operator==(const GraphEditorNodePinLookup& lhs, const GraphEditorNodePinLookup& rhs)
        {
            return lhs.node == rhs.node && lhs.name == rhs.name;
        }
    };

    template <>
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
        RID                           rid{};
        TypeHandler*                  typeHandler{};
        FunctionHandler*              functionHandler{};
        Vec2                          position{};
        String                        label{};
        String                        name{};
        bool                          initialized{};
        UniquePtr<GraphEditorNodePin> addInputPin{};
        UniquePtr<GraphEditorNodePin> addOutputPin{};

        Array<GraphEditorNodePin*> inputs{};
        Array<GraphEditorNodePin*> outputs{};
    };

    struct GraphEditorLink
    {
        RID                 rid{};
        GraphEditorNodePin* inputPin{};
        GraphEditorNodePin* outputPin{};
        TypeID              linkType{};
    };

    using GraphNodeMap = HashMap<RID, UniquePtr<GraphEditorNode>>;
    using GraphNodePinMap = HashMap<GraphEditorNodePinLookup, UniquePtr<GraphEditorNodePin>>;
    using GraphNodeLinkMap = HashMap<RID, UniquePtr<GraphEditorLink>>;

    class GraphEditor
    {
    public:
        explicit GraphEditor(AssetTree& assetTree) : m_assetTree(assetTree)
        {
        }

        FY_NO_COPY_CONSTRUCTOR(GraphEditor)

        void              OpenGraph(RID rid);
        bool              IsGraphLoaded() const;
        GraphNodeMap&     GetNodes() { return m_nodes; }
        GraphNodePinMap&  GetPins() { return m_pins; }
        GraphNodeLinkMap& GetLinks() { return m_links; }

        void AddInputNode(const Vec2& position);
        void AddOutput(TypeHandler* outputType, const Vec2& position);
        void AddNode(FunctionHandler* functionHandler, const Vec2& position);
        bool ValidateLink(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin);
        bool ValidateNewInput(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin);
        void AddNewInput(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin);
        void AddLink(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin);
        void SetPinValue(GraphEditorNodePin* input, ConstPtr value);

        void DeleteLink(RID link);
        void DeleteNode(RID node);

        void MoveNode(GraphEditorNode* node, Vec2 newPos);

    private:
        AssetTree& m_assetTree;
        RID        m_asset{};
        RID        m_graph{};
        TypeID     m_graphTypeId{};
        String     m_graphName{};

        GraphNodeMap    m_nodes;
        GraphNodePinMap m_pins;

        GraphNodeLinkMap m_links;

        void AddNodeCache(RID node);
        void AddLinkCache(RID link);

        GraphEditorNodePin* GetPin(GraphEditorNodePin* left, GraphEditorNodePin* right, GraphEditorPinKind disiredKind);

        GraphEditorNodePin* FindPin(RID node, const StringView& pin, GraphEditorPinKind graphEditorPinKind);
        void                DeletePin(GraphEditorNodePin* pin);

        void AddOutputPinCache(RID value, GraphEditorNode* node);
    };
}
