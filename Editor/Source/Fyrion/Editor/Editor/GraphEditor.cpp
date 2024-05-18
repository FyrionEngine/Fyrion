#include "GraphEditor.hpp"

#include "Fyrion/Assets/AssetTypes.hpp"
#include "Fyrion/Core/GraphTypes.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceGraph.hpp"
#include "Fyrion/Resource/ResourceObject.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/Resource/AssetTree.hpp"

namespace Fyrion
{
    void GraphEditor::AddNodeCache(RID node)
    {
        ResourceObject nodeObject = Repository::Read(node);


        GraphEditorNode* graphEditorNode = m_nodes.Emplace(node, MakeUnique<GraphEditorNode>(GraphEditorNode{
                                                               .rid = node,
                                                               .typeHandler = Registry::FindTypeByName(nodeObject[GraphNodeAsset::NodeOutput].Value<String>()),
                                                               .functionHandler = Registry::FindFunctionByName(nodeObject[GraphNodeAsset::NodeFunction].Value<String>()),
                                                               .position = nodeObject[GraphNodeAsset::Position].Value<Vec2>(),
                                                               .label = nodeObject[GraphNodeAsset::Label].Value<String>(),
                                                               .initialized = false
                                                           })).first->second.Get();

        if (graphEditorNode->functionHandler)
        {
            if (graphEditorNode->label.Empty())
            {
                graphEditorNode->label = graphEditorNode->functionHandler->GetSimpleName();
            }

            Span<ParamHandler> params = graphEditorNode->functionHandler->GetParams();
            for (const ParamHandler& param : params)
            {
                if (param.HasAttribute<GraphInput>())
                {
                    GraphEditorNodePin* pin = m_pins.Emplace(GraphEditorNodePinLookup{
                                                                 .node = node,
                                                                 .name = param.GetName()
                                                             }, MakeUnique<GraphEditorNodePin>(GraphEditorNodePin{
                                                                 .node = node,
                                                                 .label = FormatName(param.GetName()),
                                                                 .name = param.GetName(),
                                                                 .typeId = param.GetFieldInfo().typeInfo.typeId,
                                                                 .kind = GraphEditorPinKind::Input
                                                             })).first->second.Get();

                    graphEditorNode->inputs.EmplaceBack(pin);
                }
            }

            for (const ParamHandler& param : params)
            {
                if (param.HasAttribute<GraphOutput>())
                {
                    GraphEditorNodePin* pin = m_pins.Emplace(GraphEditorNodePinLookup{
                                                                 .node = node,
                                                                 .name = param.GetName(),
                                                             }, MakeUnique<GraphEditorNodePin>(GraphEditorNodePin{
                                                                 .node = node,
                                                                 .label = FormatName(param.GetName()),
                                                                 .name = param.GetName(),
                                                                 .typeId = param.GetFieldInfo().typeInfo.typeId,
                                                                 .kind = GraphEditorPinKind::Output
                                                             })).first->second.Get();
                    graphEditorNode->outputs.EmplaceBack(pin);
                }
            }
        }
        else if (graphEditorNode->typeHandler)
        {
            if (graphEditorNode->label.Empty())
            {
                graphEditorNode->label = graphEditorNode->typeHandler->GetSimpleName();
            }

            Span<FieldHandler*> fields = graphEditorNode->typeHandler->GetFields();
            for (FieldHandler* field : fields)
            {
                if (field->HasAttribute<GraphInput>())
                {

                    GraphEditorNodePin* pin = m_pins.Emplace(GraphEditorNodePinLookup{
                                                                 .node = node,
                                                                 .name = field->GetName(),
                                                             }, MakeUnique<GraphEditorNodePin>(GraphEditorNodePin{
                                                                 .node = node,
                                                                 .label = FormatName(field->GetName()),
                                                                 .name = field->GetName(),
                                                                 .typeId = field->GetFieldInfo().typeInfo.typeId,
                                                                 .kind = GraphEditorPinKind::Input
                                                             })).first->second.Get();

                    graphEditorNode->inputs.EmplaceBack(pin);
                }
            }
        }
        else
        {
            if (graphEditorNode->label.Empty())
            {
                graphEditorNode->label = "Input";
            }
        }
    }

    void GraphEditor::AddLinkCache(RID link)
    {
        ResourceObject      linkObject = Repository::Read(link);
        GraphEditorNodePin* inputPin = FindPin(linkObject[GraphNodeLinkAsset::InputNode].Value<RID>(), linkObject[GraphNodeLinkAsset::InputPin].Value<String>(), GraphEditorPinKind::Input);
        GraphEditorNodePin* outputPin = FindPin(linkObject[GraphNodeLinkAsset::OutputNode].Value<RID>(), linkObject[GraphNodeLinkAsset::OutputPin].Value<String>(), GraphEditorPinKind::Output);

        if (!inputPin || !outputPin)
        {
            return;
        }

        GraphEditorLink* editorLink = m_links.Emplace(link, MakeUnique<GraphEditorLink>(GraphEditorLink{
            .rid = link,
            .index = m_linksArray.Size(),
            .inputPin = inputPin,
            .outputPin = outputPin,
            .linkType = outputPin->typeId
        })).first->second.Get();

        m_linksArray.EmplaceBack(editorLink);

        if (const auto& it = m_nodes.Find(inputPin->node))
        {
            it->second->links.Insert(link);
        }

        if (const auto& it = m_nodes.Find(outputPin->node))
        {
            it->second->links.Insert(link);
        }
    }

    GraphEditorNodePin* GraphEditor::GetPin(GraphEditorNodePin* left, GraphEditorNodePin* right, GraphEditorPinKind disiredKind)
    {
        if (left->kind == disiredKind) return left;
        if (right->kind == disiredKind) return right;
        return nullptr;
    }

    GraphEditorNodePin* GraphEditor::FindPin(RID node, const StringView& pin, GraphEditorPinKind graphEditorPinKind)
    {
        if (auto it = m_pins.Find(GraphEditorNodePinLookup{
            .node = node,
            .name = pin
        }))
        {
            GraphEditorNodePin* pin = it->second.Get();
            if (pin->kind == graphEditorPinKind)
            {
                return pin;
            }
        }
        return nullptr;
    }


    void GraphEditor::OpenGraph(RID rid)
    {
        m_nodes.Clear();
        m_pins.Clear();

        m_asset = rid;

        ResourceObject asset = Repository::Read(m_asset);
        m_graph = asset[Asset::Object].Value<RID>();
        m_graphName = asset[Asset::Name].Value<String>();
        m_graphTypeId = Repository::GetResourceTypeId(Repository::GetResourceType(m_graph));

        ResourceObject object = Repository::Read(m_graph);

        Array<RID> nodes = object.GetSubObjectSetAsArray(ResourceGraphAsset::Nodes);
        for (RID node : nodes)
        {
            AddNodeCache(node);
        }

        Array<RID> links = object.GetSubObjectSetAsArray(ResourceGraphAsset::Links);
        for (RID link : links)
        {
            AddLinkCache(link);
        }
    }

    void GraphEditor::AddOutput(TypeHandler* outputType, const Vec2& position)
    {
        RID nodeAsset = Repository::CreateResource<GraphNodeAsset>();

        ResourceObject nodeObject = Repository::Write(nodeAsset);
        nodeObject[GraphNodeAsset::NodeOutput] = outputType->GetName();
        nodeObject[GraphNodeAsset::Position] = position;
        nodeObject.Commit();

        ResourceObject graphAsset = Repository::Write(m_graph);
        graphAsset.AddToSubObjectSet(ResourceGraphAsset::Nodes, nodeAsset);
        graphAsset.Commit();

        Repository::SetUUID(nodeAsset, UUID::RandomUUID());

        AddNodeCache(nodeAsset);

        m_assetTree.MarkDirty();
    }

    void GraphEditor::AddNode(FunctionHandler* functionHandler, const Vec2& position)
    {
        RID nodeAsset = Repository::CreateResource<GraphNodeAsset>();

        ResourceObject nodeObject = Repository::Write(nodeAsset);
        nodeObject[GraphNodeAsset::NodeFunction] = functionHandler->GetName();
        nodeObject[GraphNodeAsset::Position] = position;
        nodeObject.Commit();

        ResourceObject graphAsset = Repository::Write(m_graph);
        graphAsset.AddToSubObjectSet(ResourceGraphAsset::Nodes, nodeAsset);
        graphAsset.Commit();

        Repository::SetUUID(nodeAsset, UUID::RandomUUID());

        AddNodeCache(nodeAsset);

        m_assetTree.MarkDirty();
    }

    bool GraphEditor::ValidateLink(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin)
    {
        GraphEditorNodePin* input = GetPin(inputPin, outputPin, GraphEditorPinKind::Input);
        GraphEditorNodePin* output = GetPin(inputPin, outputPin, GraphEditorPinKind::Output);
        return input && output && input->kind != output->kind && input->typeId == output->typeId;
    }

    void GraphEditor::AddLink(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin)
    {
        GraphEditorNodePin* input = GetPin(inputPin, outputPin, GraphEditorPinKind::Input);
        GraphEditorNodePin* output = GetPin(inputPin, outputPin, GraphEditorPinKind::Output);

        RID linkAsset = Repository::CreateResource<GraphNodeLinkAsset>();

        ResourceObject linkObject = Repository::Write(linkAsset);
        linkObject[GraphNodeLinkAsset::InputNode] = input->node;
        linkObject[GraphNodeLinkAsset::InputPin] = input->name;
        linkObject[GraphNodeLinkAsset::OutputNode] = output->node;
        linkObject[GraphNodeLinkAsset::OutputPin] = output->name;
        linkObject.Commit();

        ResourceObject graphAsset = Repository::Write(m_graph);
        graphAsset.AddToSubObjectSet(ResourceGraphAsset::Links, linkAsset);
        graphAsset.Commit();

        Repository::SetUUID(linkAsset, UUID::RandomUUID());

        m_assetTree.MarkDirty();

        AddLinkCache(linkAsset);
    }

    void GraphEditor::DeleteLink(RID link)
    {
        Repository::DestroyResource(link);
        m_links.Erase(link);
        m_assetTree.MarkDirty();
    }

    void GraphEditor::DeleteNode(RID node)
    {
        Repository::DestroyResource(node);
        if (auto it = m_nodes.Find(node))
        {
            ForEach(it->second->inputs.begin(), it->second->inputs.end(), this, &GraphEditor::DeletePin);
            ForEach(it->second->outputs.begin(), it->second->outputs.end(), this, &GraphEditor::DeletePin);

            m_nodes.Erase(it);
        }
        m_assetTree.MarkDirty();
    }

    void GraphEditor::MoveNode(GraphEditorNode* node, Vec2 newPos)
    {
        node->position = newPos;

        ResourceObject nodeObject = Repository::Write(node->rid);
        nodeObject[GraphNodeAsset::Position] = node->position;
        nodeObject.Commit();

        m_assetTree.MarkDirty();
    }

    void GraphEditor::DeletePin(GraphEditorNodePin* pin)
    {
        m_pins.Erase(GraphEditorNodePinLookup{
            .node = pin->node,
            .name = pin->name,
        });
    }

    bool GraphEditor::IsGraphLoaded() const
    {
        return m_graph;
    }
}
