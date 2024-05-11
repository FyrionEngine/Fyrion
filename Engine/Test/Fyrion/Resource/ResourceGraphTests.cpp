#include "doctest.h"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Assets/AssetTypes.hpp"
#include "Fyrion/Core/GraphTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceGraph.hpp"

namespace Fyrion
{
    struct GraphNodeAsset;
}

using namespace Fyrion;

namespace
{
    struct TestOutputNode
    {
        String stringValue;
        i32    intValue;
        u32    stringSize;

        static void RegisterType(NativeTypeHandler<TestOutputNode>& type)
        {
            type.Field<&TestOutputNode::stringValue>("stringValue").Attribute<GraphInput>();
            type.Field<&TestOutputNode::intValue>("intValue").Attribute<GraphInput>();
            type.Field<&TestOutputNode::stringSize>("stringSize").Attribute<GraphInput>();
            type.Attribute<ResourceGraphOutput>();
        }
    };


    void MultypleValue(u32 vl1, u32 vl2, u32& out)
    {
        out = vl1 * vl2;
    }

    void ConcatString(const String& input1, const String& input2, String& out)
    {
        out = input1 + input2;
    }

    void StringSize(const String& string, u32& size)
    {
        size = string.Size();
    }

    void RegisterTypes()
    {
        Registry::Type<TestOutputNode>("Fyrion::TestOutputNode");

        auto multiplyValue = Registry::Function<MultypleValue>("Fyrion::MultypleValue");
        multiplyValue.Param<0>("vl1").Attribute<GraphInput>();
        multiplyValue.Param<1>("vl2").Attribute<GraphInput>();
        multiplyValue.Param<2>("out").Attribute<GraphOutput>();
        multiplyValue.Attribute<ResourceGraphNode>();

        auto concatString = Registry::Function<ConcatString>("Fyrion::ConcatString");
        concatString.Param<0>("input1").Attribute<GraphInput>();
        concatString.Param<1>("input2").Attribute<GraphInput>();
        concatString.Param<2>("out").Attribute<GraphOutput>();
        concatString.Attribute<ResourceGraphNode>();

        auto stringSize = Registry::Function<StringSize>("Fyrion::StringSize");
        stringSize.Param<0>("string").Attribute<GraphInput>();
        stringSize.Param<1>("size").Attribute<GraphOutput>();
        stringSize.Attribute<ResourceGraphNode>();
    }


    TEST_CASE("Resource::ResourceGraphAssetBasics")
    {
        Engine::Init();
        {
            RegisterTypes();
            RID rid = Repository::CreateResource<ResourceGraphAsset>();

            //asset
            {
                RID contentString1 = Repository::CreateResource<GraphNodeAsset>();
                {
                    RID vl1 = Repository::CreateResource<GraphNodeValue>();
                    {
                        RID    value = Repository::CreateResource<String>();
                        String vl = "valuevalue1";
                        Repository::Commit(value, &vl);
                        ResourceObject object = Repository::Write(vl1);
                        object[GraphNodeValue::Input] = "input1"; //TODO string?
                        object.SetSubObject(GraphNodeValue::Value, value);
                        object.Commit();
                    }

                    RID vl2 = Repository::CreateResource<GraphNodeValue>();

                    {
                        RID    value = Repository::CreateResource<String>();
                        String vl = "valuevalue2";
                        Repository::Commit(value, &vl);
                        ResourceObject object = Repository::Write(vl2);
                        object[GraphNodeValue::Input] = "input2";
                        object.SetSubObject(GraphNodeValue::Value, value);
                        object.Commit();
                    }


                    ResourceObject object = Repository::Write(contentString1);
                    object[GraphNodeAsset::NodeType] = "Fyrion::ConcatString";
                    object.AddToSubObjectSet(GraphNodeAsset::InputValues, vl1);
                    object.AddToSubObjectSet(GraphNodeAsset::InputValues, vl2);
                    object.Commit();
                }


                RID output = Repository::CreateResource<GraphNodeAsset>();
                {
                    ResourceObject object = Repository::Write(output);
                    object[GraphNodeAsset::NodeType] = "Fyrion::TestOutputNode";
                    object.Commit();
                }

                RID link1 = Repository::CreateResource<GraphNodeLinkAsset>();
                {
                    ResourceObject object = Repository::Write(link1);
                    object[GraphNodeLinkAsset::InputNode] = contentString1;
                    object[GraphNodeLinkAsset::InputPin] = "out";
                    object[GraphNodeLinkAsset::OutputNode] = output;
                    object[GraphNodeLinkAsset::OutputPin] = "stringValue";
                    object.Commit();
                }

                {
                    ResourceObject object = Repository::Write(rid);
                    object.AddToSubObjectSet(ResourceGraphAsset::Nodes, contentString1);
                    object.AddToSubObjectSet(ResourceGraphAsset::Nodes, output);
                    object.AddToSubObjectSet(ResourceGraphAsset::Links, link1);
                    object.Commit();
                }
            }
            //TODO make tests with assets
        }
        Engine::Destroy();
    }

    TEST_CASE("Resource::ResourceGraphBasics")
    {
        Engine::Init();
        {
            String vlInput = "value";
            u32    defaultValue = 3;

            RegisterTypes();

            FixedArray<ResourceGraphNodeValue, 2> inputValues = {
                ResourceGraphNodeValue{.name = "inputValue", .typeHandler = Registry::FindType<String>(), .publicValue = true},
                ResourceGraphNodeValue{.name = "valueToSum", .typeHandler = Registry::FindType<u32>(), .value = &defaultValue, .publicValue = true},
            };

            FixedArray<ResourceGraphNodeInfo, 5> nodes{
                ResourceGraphNodeInfo{.id = 0, .values = inputValues},
                ResourceGraphNodeInfo{.id = 4, .typeHandler = Registry::FindType<TestOutputNode>()},
                ResourceGraphNodeInfo{
                    .id = 1, .functionHandler = Registry::FindFunctionByName("Fyrion::ConcatString"), .values = {
                        ResourceGraphNodeValue{.name = "input2", .typeHandler = Registry::FindType<String>(), .value = &vlInput}
                    }
                },
                ResourceGraphNodeInfo{.id = 2, .functionHandler = Registry::FindFunctionByName("Fyrion::StringSize"),},
                ResourceGraphNodeInfo{.id = 3, .functionHandler = Registry::FindFunctionByName("Fyrion::MultypleValue"),},
            };

            FixedArray<ResourceGraphLinkInfo, 6> links{
                ResourceGraphLinkInfo{.outputNodeId = 0, .outputPin = "inputValue", .inputNodeId = 1, .inputPin = "input1",},
                ResourceGraphLinkInfo{.outputNodeId = 0, .outputPin = "valueToSum", .inputNodeId = 3, .inputPin = "vl1",},
                ResourceGraphLinkInfo{.outputNodeId = 1, .outputPin = "out", .inputNodeId = 2, .inputPin = "string",},
                ResourceGraphLinkInfo{.outputNodeId = 1, .outputPin = "out", .inputNodeId = 4, .inputPin = "stringValue",},
                ResourceGraphLinkInfo{.outputNodeId = 2, .outputPin = "size", .inputNodeId = 3, .inputPin = "vl2",},
                ResourceGraphLinkInfo{.outputNodeId = 3, .outputPin = "out", .inputNodeId = 4, .inputPin = "stringSize",}
            };

            ResourceGraph resourceGraph = {nodes, links};

            auto graphNodes = resourceGraph.GetNodes();
            CHECK(graphNodes[0]->GetId() == 0);
            CHECK(graphNodes[1]->GetId() == 1);
            CHECK(graphNodes[2]->GetId() == 2);
            CHECK(graphNodes[3]->GetId() == 3);
            CHECK(graphNodes[4]->GetId() == 4);

            ResourceGraphInstance* instance = resourceGraph.CreateInstance();
            String vl = "default-";

            instance->SetInputValue("inputValue", &vl);
            instance->Execute();

            Span<ConstPtr> outputs = instance->GetOutputs(GetTypeID<TestOutputNode>());
            REQUIRE(!outputs.Empty());
            REQUIRE(outputs[0]);

            const TestOutputNode* testOutputNode = static_cast<const TestOutputNode*>(outputs[0]);
            CHECK(testOutputNode->stringValue == "default-value");
            CHECK(testOutputNode->stringSize == testOutputNode->stringValue.Size() * defaultValue);

            instance->Destroy();
        }
        Engine::Destroy();
    }
}
