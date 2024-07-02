#if FY_ASSET_REFACTOR

#include <algorithm>
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

    void GraphEditor::AddOutputPinCache(RID value, GraphEditorNode* node)
    {
        ResourceObject valueObject = Repository::Read(value);

        String name = valueObject[GraphNodeValue::Name].Value<String>();
        String typeName = valueObject[GraphNodeValue::Type].Value<String>();
        String resourceTypeName = valueObject[GraphNodeValue::ResourceType].Value<String>();

        TypeID typeId = 0;
        TypeID resourceTypeId = 0;

        if (TypeHandler* typeHandler = Registry::FindTypeByName(typeName))
        {
            typeId = typeHandler->GetTypeInfo().typeId;
        }

        if (TypeHandler* typeHandler = Registry::FindTypeByName(resourceTypeName))
        {
            resourceTypeId = typeHandler->GetTypeInfo().typeId;
        }
        else if (ResourceType* resourceType = Repository::GetResourceTypeByName(resourceTypeName))
        {
            resourceTypeId = Repository::GetResourceTypeId(resourceType);
        }


        GraphEditorNodePinLookup lookup = GraphEditorNodePinLookup{
            .node = node->rid,
            .name = name,
        };

        GraphEditorNodePin* pin = m_pins.Emplace(lookup, MakeUnique<GraphEditorNodePin>(GraphEditorNodePin{
                                                     .node = node,
                                                     .label = name,
                                                     .name = name,
                                                     .typeId = typeId,
                                                     .valueAsset = value,
                                                     .kind = GraphEditorPinKind::Output,
                                                     .publicValue = valueObject[GraphNodeValue::PublicValue].Value<bool>(),
                                                     .resourceType = resourceTypeId,
                                                     .editable = true
                                                 })).first->second.Get();

        node->outputs.EmplaceBack(pin);
    }

    void GraphEditor::AddNodeCache(RID node)
    {
        ResourceObject nodeObject = Repository::Read(node);

        Array<RID> inputValues = nodeObject.GetSubObjectSetAsArray(GraphNodeAsset::InputValues);
        HashMap<String, Pair<RID, RID>> inputCache;

        for (RID input : inputValues)
        {
            ResourceObject valueObject = Repository::Read(input);
            if (valueObject)
            {
                inputCache.Insert(valueObject[GraphNodeValue::Name].Value<String>(), MakePair(input, valueObject[GraphNodeValue::Value].Value<RID>()));
            }
        }

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
            graphEditorNode->name = graphEditorNode->functionHandler->GetSimpleName();
            if (graphEditorNode->label.Empty())
            {
                graphEditorNode->label = FormatName(graphEditorNode->name);
            }

            Span<ParamHandler> params = graphEditorNode->functionHandler->GetParams();
            for (const ParamHandler& param : params)
            {
                if (param.HasAttribute<GraphInput>())
                {
                    const GraphInput* graphInput = param.GetAttribute<GraphInput>();

                    RID value = {};
                    RID valueAsset = {};

                    if (auto itValue = inputCache.Find(param.GetName()))
                    {
                        value = itValue->second.second;
                        valueAsset = itValue->second.first;
                    }

                    GraphEditorNodePin* pin = m_pins.Emplace(GraphEditorNodePinLookup{
                                                                 .node = node,
                                                                 .name = param.GetName()
                                                             }, MakeUnique<GraphEditorNodePin>(GraphEditorNodePin{
                                                                 .node = graphEditorNode,
                                                                 .label = FormatName(param.GetName()),
                                                                 .name = param.GetName(),
                                                                 .typeId = param.GetFieldInfo().typeInfo.typeId,
                                                                 .value = value,
                                                                 .valueAsset = valueAsset,
                                                                 .kind = GraphEditorPinKind::Input,
                                                                 .resourceType = graphInput->typeId
                                                             })).first->second.Get();

                    graphEditorNode->inputs.EmplaceBack(pin);
                }
            }

            for (const ParamHandler& param : params)
            {
                if (param.HasAttribute<GraphOutput>())
                {
                    const GraphOutput* graphOutput = param.GetAttribute<GraphOutput>();


                    GraphEditorNodePin* pin = m_pins.Emplace(GraphEditorNodePinLookup{
                                                                 .node = node,
                                                                 .name = param.GetName(),
                                                             }, MakeUnique<GraphEditorNodePin>(GraphEditorNodePin{
                                                                 .node = graphEditorNode,
                                                                 .label = FormatName(param.GetName()),
                                                                 .name = param.GetName(),
                                                                 .typeId = param.GetFieldInfo().typeInfo.typeId,
                                                                 .kind = GraphEditorPinKind::Output,
                                                                 .resourceType = graphOutput->typeId
                                                             })).first->second.Get();

                    graphEditorNode->outputs.EmplaceBack(pin);
                }
            }
        }
        else if (graphEditorNode->typeHandler)
        {
            graphEditorNode->name = graphEditorNode->typeHandler->GetSimpleName();
            if (graphEditorNode->label.Empty())
            {
                graphEditorNode->label = FormatName(graphEditorNode->name);
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
                                                                 .node = graphEditorNode,
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
            graphEditorNode->addOutputPin = MakeUnique<GraphEditorNodePin>(GraphEditorNodePin{
                .node = graphEditorNode,
                .kind = GraphEditorPinKind::AddOutputPin
            });

            for (RID inputValue : inputValues)
            {
                AddOutputPinCache(inputValue, graphEditorNode);
            }

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

        m_links.Emplace(link, MakeUnique<GraphEditorLink>(GraphEditorLink{
            .rid = link,
            .inputPin = inputPin,
            .outputPin = outputPin,
            .linkType = outputPin->typeId
        }));

        inputPin->links.Emplace(link);
        outputPin->links.Emplace(link);
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

        Array<RID> nodes = object.GetSubObjectSetAsArray(GraphAsset::Nodes);
        for (RID node : nodes)
        {
            AddNodeCache(node);
        }

        Array<RID> links = object.GetSubObjectSetAsArray(GraphAsset::Links);
        for (RID link : links)
        {
            AddLinkCache(link);
        }
    }

    void GraphEditor::AddInputNode(const Vec2& position)
    {
        RID nodeAsset = Repository::CreateResource<GraphNodeAsset>();

        ResourceObject nodeObject = Repository::Write(nodeAsset);
        nodeObject[GraphNodeAsset::Position] = position;
        nodeObject.Commit();

        ResourceObject graphAsset = Repository::Write(m_graph);
        graphAsset.AddToSubObjectSet(GraphAsset::Nodes, nodeAsset);
        graphAsset.Commit();

        Repository::SetUUID(nodeAsset, UUID::RandomUUID());
        AddNodeCache(nodeAsset);
        m_assetTree.MarkDirty();
    }

    void GraphEditor::AddOutput(TypeHandler* outputType, const Vec2& position)
    {
        RID nodeAsset = Repository::CreateResource<GraphNodeAsset>();

        ResourceObject nodeObject = Repository::Write(nodeAsset);
        nodeObject[GraphNodeAsset::NodeOutput] = outputType->GetName();
        nodeObject[GraphNodeAsset::Position] = position;
        nodeObject.Commit();

        ResourceObject graphAsset = Repository::Write(m_graph);
        graphAsset.AddToSubObjectSet(GraphAsset::Nodes, nodeAsset);
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
        graphAsset.AddToSubObjectSet(GraphAsset::Nodes, nodeAsset);
        graphAsset.Commit();

        Repository::SetUUID(nodeAsset, UUID::RandomUUID());

        AddNodeCache(nodeAsset);

        m_assetTree.MarkDirty();
    }

    bool GraphEditor::ValidateLink(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin)
    {
        GraphEditorNodePin* input = GetPin(inputPin, outputPin, GraphEditorPinKind::Input);
        GraphEditorNodePin* output = GetPin(inputPin, outputPin, GraphEditorPinKind::Output);
        return input && output && input->typeId == output->typeId;
    }

    bool GraphEditor::ValidateNewInput(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin)
    {
        GraphEditorNodePin* input = GetPin(inputPin, outputPin, GraphEditorPinKind::Input);
        GraphEditorNodePin* addOutput = GetPin(inputPin, outputPin, GraphEditorPinKind::AddOutputPin);


        if (input && addOutput)
        {
            for(auto node : addOutput->node->outputs)
            {
                if (node->name == input->name)
                {
                    return false;
                }
            }
            return true;
        }

        return false;
    }

    void GraphEditor::AddNewInput(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin)
    {
        GraphEditorNodePin* input = GetPin(inputPin, outputPin, GraphEditorPinKind::Input);
        GraphEditorNodePin* addOutput = GetPin(inputPin, outputPin, GraphEditorPinKind::AddOutputPin);

        if (input && addOutput)
        {
            RID valueAsset = Repository::CreateResource<GraphNodeValue>();
            Repository::SetUUID(valueAsset, UUID::RandomUUID());


            ResourceObject valueObject = Repository::Write(valueAsset);
            valueObject[GraphNodeValue::Name] = input->name;
            valueObject[GraphNodeValue::PublicValue] = true;
            valueObject[GraphNodeValue::Type] = Registry::FindTypeById(input->typeId)->GetName();
            if (TypeHandler* resourceTypeHandler = Registry::FindTypeById(input->resourceType))
            {
                valueObject[GraphNodeValue::ResourceType] = resourceTypeHandler->GetName();
            }
            else if (ResourceType* resourceType = Repository::GetResourceTypeById(input->resourceType))
            {
                valueObject[GraphNodeValue::ResourceType] = Repository::GetResourceTypeName(resourceType);
            }

            valueObject.Commit();

            ResourceObject nodeObject = Repository::Write(addOutput->node->rid);
            nodeObject.AddToSubObjectSet(GraphNodeAsset::InputValues, valueAsset);
            nodeObject.Commit();

            AddOutputPinCache(valueAsset, addOutput->node);

            RID linkAsset = Repository::CreateResource<GraphNodeLinkAsset>();

            ResourceObject linkObject = Repository::Write(linkAsset);
            linkObject[GraphNodeLinkAsset::InputNode] = input->node->rid;
            linkObject[GraphNodeLinkAsset::InputPin] = input->name;
            linkObject[GraphNodeLinkAsset::OutputNode] = addOutput->node->rid;
            linkObject[GraphNodeLinkAsset::OutputPin] = input->name;
            linkObject.Commit();

            ResourceObject graphAsset = Repository::Write(m_graph);
            graphAsset.AddToSubObjectSet(GraphAsset::Links, linkAsset);
            graphAsset.Commit();

            Repository::SetUUID(linkAsset, UUID::RandomUUID());

            AddLinkCache(linkAsset);
            m_assetTree.MarkDirty();
        }
    }

    void GraphEditor::AddLink(GraphEditorNodePin* inputPin, GraphEditorNodePin* outputPin)
    {
        GraphEditorNodePin* input = GetPin(inputPin, outputPin, GraphEditorPinKind::Input);
        GraphEditorNodePin* output = GetPin(inputPin, outputPin, GraphEditorPinKind::Output);

        RID linkAsset = Repository::CreateResource<GraphNodeLinkAsset>();

        ResourceObject linkObject = Repository::Write(linkAsset);
        linkObject[GraphNodeLinkAsset::InputNode] = input->node->rid;
        linkObject[GraphNodeLinkAsset::InputPin] = input->name;
        linkObject[GraphNodeLinkAsset::OutputNode] = output->node->rid;
        linkObject[GraphNodeLinkAsset::OutputPin] = output->name;
        linkObject.Commit();

        ResourceObject graphAsset = Repository::Write(m_graph);
        graphAsset.AddToSubObjectSet(GraphAsset::Links, linkAsset);
        graphAsset.Commit();

        Repository::SetUUID(linkAsset, UUID::RandomUUID());

        AddLinkCache(linkAsset);

        m_assetTree.MarkDirty();

    }

    void GraphEditor::SetPinValue(GraphEditorNodePin* input, ConstPtr value)
    {
        if (!input->value)
        {
            input->value = Repository::CreateResource(input->typeId);
            Repository::SetUUID(input->value, UUID::RandomUUID());
            input->valueAsset = Repository::CreateResource<GraphNodeValue>();
            Repository::SetUUID(input->valueAsset, UUID::RandomUUID());

            ResourceObject valueObject = Repository::Write(input->valueAsset);
            valueObject[GraphNodeValue::Name] = input->name;
            valueObject.SetSubObject(GraphNodeValue::Value, input->value);
            valueObject.Commit();

            ResourceObject nodeObject = Repository::Write(input->node->rid);
            nodeObject.AddToSubObjectSet(GraphNodeAsset::InputValues, input->valueAsset);
            nodeObject.Commit();
        }
        Repository::Commit(input->value, value);

        m_assetTree.MarkDirty();
    }

    void GraphEditor::DeleteLink(RID link)
    {
        Repository::DestroyResource(link);

        if (const auto& it = m_links.Find(link))
        {
            it->second->inputPin->links.Erase(link);
            it->second->outputPin->links.Erase(link);
        }

        m_links.Erase(link);
        m_assetTree.MarkDirty();
    }

    void GraphEditor::DeleteNode(RID node)
    {
        Repository::DestroyResource(node);
        if (auto it = m_nodes.Find(node))
        {
            ForEach(it->second->inputs.begin(), it->second->inputs.end(), this, &GraphEditor::DeletePinCache);
            ForEach(it->second->outputs.begin(), it->second->outputs.end(), this, &GraphEditor::DeletePinCache);

            m_nodes.Erase(it);
        }
        m_assetTree.MarkDirty();
    }

    void GraphEditor::DeletePin(GraphEditorNodePin* pin)
    {
        for(const auto& it: pin->links)
        {
            m_links.Erase(it.first);
            Repository::DestroyResource(it.first);
        }

        if (const auto it = std::find(pin->node->inputs.begin(), pin->node->inputs.end(), pin); it != pin->node->inputs.end())
        {
            pin->node->inputs.Erase(it);
        }

        if (const auto it = std::find(pin->node->outputs.begin(), pin->node->outputs.end(), pin); it != pin->node->outputs.end())
        {
            pin->node->outputs.Erase(it);
        }

        Repository::DestroyResource(pin->valueAsset);

        DeletePinCache(pin);
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

    void GraphEditor::RenameNode(GraphEditorNode* node, StringView newLabel)
    {
        node->label = newLabel;

        ResourceObject nodeObject = Repository::Write(node->rid);
        nodeObject[GraphNodeAsset::Label] = newLabel;
        nodeObject.Commit();

        m_assetTree.MarkDirty();
    }

    void GraphEditor::RenamePin(GraphEditorNodePin* pin, StringView newName)
    {
        // pin->name = newName;
        // pin->label = newName;
    }

    void GraphEditor::ChangePinVisibility(GraphEditorNodePin* pin, bool publicValue)
    {
        pin->publicValue = publicValue;

        ResourceObject valueObject = Repository::Write(pin->valueAsset);
        valueObject[GraphNodeValue::PublicValue] = publicValue;
        valueObject.Commit();

        m_assetTree.MarkDirty();
    }

    GraphEditorNode* GraphEditor::GetNodeByRID(RID rid)
    {
        if (const auto it = m_nodes.Find(rid))
        {
            return it->second.Get();
        }
        return nullptr;
    }

    void GraphEditor::DeletePinCache(GraphEditorNodePin* pin)
    {
        m_pins.Erase(GraphEditorNodePinLookup{
            .node = pin->node->rid,
            .name = pin->name,
        });
    }

    bool GraphEditor::IsGraphLoaded() const
    {
        return m_graph;
    }
}

#endif