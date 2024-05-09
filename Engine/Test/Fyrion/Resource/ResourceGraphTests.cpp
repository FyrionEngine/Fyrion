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
            type.Field<&TestOutputNode::stringValue>("stringValue");
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
        sumValue.Param<2>("out").Attribute<GraphInput>();
        sumValue.Attribute<ResourceGraphNode>();

        auto concatString = Registry::Function<ConcatString>("Fyrion::ConcatString");
        concatString.Param<0>("input1").Attribute<GraphInput>();
        concatString.Param<1>("input2").Attribute<GraphInput>();
        concatString.Param<2>("out").Attribute<GraphInput>();
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

        }
        Engine::Destroy();
    }

    TEST_CASE("Resource::ResourceGraphBasics")
    {
        String vl = "value-";

        Engine::Init();
        {
            RegisterTypes();

            FixedArray<ResourceGraphInputNodeInfo, 1> inputs{
                ResourceGraphInputNodeInfo{
                    .index = 0,
                    .name = "inputValue",
                    .typeHandler = Registry::FindType<String>(),
                    .value = nullptr
                }
            };


            FixedArray<ResourceGraphNodeInfo, 1> nodes{
                ResourceGraphNodeInfo{
                    .index = 1,
                    .functionHandler = Registry::FindFunctionByName("Fyrion::ConcatString"),
                    .defaultValues = {
                        ResourceGraphNodeValue{
                            .name = "input2",
                            .typeHandler = Registry::FindType<String>(),
                            .value = &vl
                        }
                    }
                }
            };

            FixedArray<ResourceGraphOutputNodeInfo, 1> outputNodes{
                ResourceGraphOutputNodeInfo{
                    .index = 2,
                    .typeHandler = Registry::FindType<TestOutputNode>()
                }
            };

            FixedArray<ResourceGraphLinkInfo, 2> links{
                ResourceGraphLinkInfo{
                    .inputNodeIndex = 0,
                    .inputPin = "inputValue",
                    .outputNodeIndex = 1,
                    .outputPin = "input1",
                },
                ResourceGraphLinkInfo{
                    .inputNodeIndex = 1,
                    .inputPin = "out",
                    .outputNodeIndex = 2,
                    .outputPin = "stringValue",
                }
            };

            ResourceGraph resourceGraph = {inputs, outputNodes, nodes, links};

            ResourceGraphInstance* instance = resourceGraph.CreateInstance();
            String vl = "value-";
            instance->SetInputValue("inputValue", &vl, sizeof(String));

            instance->Execute();

            Span<TestOutputNode> outputs = instance->GetOutputs<TestOutputNode>();
            //CHECK(outputs[0].stringValue == "value-value-");
        }
        Engine::Destroy();
    }
}
