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
        i32    intValue{};

        static void RegisterType(NativeTypeHandler<TestOutputNode>& type)
        {
            type.Field<&TestOutputNode::stringValue>("stringValue").Attribute<GraphInput>();
            type.Field<&TestOutputNode::intValue>("intValue");
            type.Attribute<ResourceGraphOutput>();
        }
    };


    void SumValue(i32 vl1, i32 vl2, i32& out)
    {
        out = vl1 + vl2;
    }

    void ConcatString(const String& input1, const String& input2, String& out)
    {
        out = input1 + input2;
    }

    void RegisterTypes()
    {
        Registry::Type<TestOutputNode>("Fyrion::TestOutputNode");

        auto sumValue = Registry::Function<SumValue>("Fyrion::SumValue");
        sumValue.Param<0>("vl1").Attribute<GraphInput>();
        sumValue.Param<1>("vl2").Attribute<GraphInput>();
        sumValue.Param<2>("out").Attribute<GraphOutput>();
        sumValue.Attribute<ResourceGraphNode>();

        auto concatString = Registry::Function<ConcatString>("Fyrion::ConcatString");
        concatString.Param<0>("input1").Attribute<GraphInput>();
        concatString.Param<1>("input2").Attribute<GraphInput>();
        concatString.Param<2>("out").Attribute<GraphOutput>();
        concatString.Attribute<ResourceGraphNode>();
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
                        RID value = Repository::CreateResource<String>();
                        String vl = "valuevalue1";
                        Repository::Commit(value, &vl);
                        ResourceObject object = Repository::Write(vl1);
                        object[GraphNodeValue::Input] = "input1";  //TODO string?
                        object.SetSubObject(GraphNodeValue::Value, value);
                        object.Commit();
                    }

                    RID vl2 = Repository::CreateResource<GraphNodeValue>();

                    {
                        RID value = Repository::CreateResource<String>();
                        String vl = "valuevalue2";
                        Repository::Commit(value, &vl);
                        ResourceObject object = Repository::Write(vl2);
                        object[GraphNodeValue::Input] = "input2";
                        object.SetSubObject(GraphNodeValue::Value, value);
                        object.Commit();
                    }


                    ResourceObject object = Repository::Write(contentString1);
                    object[GraphNodeAsset::NodeType] = "Fyrion::ConcatString"; //TODO string?
                    object.AddToSubObjectSet(GraphNodeAsset::InputValues, vl1);
                    object.AddToSubObjectSet(GraphNodeAsset::InputValues, vl2);
                    object.Commit();
                }


                RID output = Repository::CreateResource<GraphNodeAsset>();
                {
                    ResourceObject object = Repository::Write(output);
                    object[GraphNodeAsset::NodeType] = "Fyrion::TestOutputNode"; //TODO string?
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
        String vl = "value";

        Engine::Init();
        {
            RegisterTypes();

            FixedArray<ResourceGraphNodeValue, 1> inputValues = {
                ResourceGraphNodeValue{
                    .name = "inputValue",
                    .typeHandler = Registry::FindType<String>(),
                    .publicValue = true
                }
            };

            FixedArray<ResourceGraphNodeInfo, 3> nodes{
                ResourceGraphNodeInfo{
                    .id = 0,
                    .values = inputValues
                }
                ,
                ResourceGraphNodeInfo{
                    .id = 2,
                    .typeHandler = Registry::FindType<TestOutputNode>()
                },
                ResourceGraphNodeInfo{
                    .id = 1,
                    .functionHandler = Registry::FindFunctionByName("Fyrion::ConcatString"),
                    .values = {
                        ResourceGraphNodeValue{
                            .name = "input2",
                            .typeHandler = Registry::FindType<String>(),
                            .value = &vl
                        }
                    }
                }
            };

            FixedArray<ResourceGraphLinkInfo, 2> links{
                ResourceGraphLinkInfo{
                    .outputNodeId = 0,
                    .outputPin = "inputValue",
                    .inputNodeId = 1,
                    .inputPin = "input1",
                },
                ResourceGraphLinkInfo{
                    .outputNodeId = 1,
                    .outputPin = "out",
                    .inputNodeId = 2,
                    .inputPin = "stringValue",
                }
            };

            ResourceGraph resourceGraph = {nodes, links};

            auto graphNodes = resourceGraph.GetNodes();
            CHECK(graphNodes[0]->GetId() == 0);
            CHECK(graphNodes[1]->GetId() == 1);
            CHECK(graphNodes[2]->GetId() == 2);

            ResourceGraphInstance* instance = resourceGraph.CreateInstance();
            String vl = "default-";
            instance->SetInputValue("inputValue", &vl);

            instance->Execute();

            Span<ConstPtr> outputs = instance->GetOutputs(GetTypeID<TestOutputNode>());
            REQUIRE(!outputs.Empty());
            REQUIRE(outputs[0]);

            const TestOutputNode* testOutputNode = static_cast<const TestOutputNode*>(outputs[0]);
            CHECK(testOutputNode->stringValue == "default-value");

            instance->Destroy();
        }
        Engine::Destroy();
    }
}
