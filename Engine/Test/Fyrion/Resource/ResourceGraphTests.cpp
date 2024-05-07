#include "doctest.h"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/GraphTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Resource/ResourceGraph.hpp"

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

    void ConcatString(const String& vl1, const String& vl2, String& out)
    {
        out = vl1 + vl2;
    }

    void RegisterTypes()
    {
        Registry::Type<TestOutputNode>();

        auto sumValue = Registry::Function<SumValue>("SumValue");
        sumValue.Param<0>("vl1").Attribute<GraphInput>();
        sumValue.Param<1>("vl2").Attribute<GraphInput>();
        sumValue.Param<2>("out").Attribute<GraphInput>();
        sumValue.Attribute<ResourceGraphNode>();


        auto concatString = Registry::Function<ConcatString>("ConcatString");
        concatString.Param<0>("vl1").Attribute<GraphInput>();
        concatString.Param<1>("vl2").Attribute<GraphInput>();
        concatString.Param<2>("out").Attribute<GraphInput>();
        concatString.Attribute<ResourceGraphNode>();
    }


    TEST_CASE("Resource::ResourceGraphBasics")
    {
        Engine::Init();
        {
            RegisterTypes();
        }
        Engine::Destroy();
    }
}
