
#include <doctest.h>
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/Resource/ResourceSerialization.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Resource/Repository.hpp"

#include <iostream>

using namespace Fyrion;

namespace
{

    struct NestedStruct
    {
        i32    value{};
        String string{};
    };

    bool operator==(const NestedStruct& a, const NestedStruct& b)
    {
        return a.string == b.string && a.value == b.value;
    }

    struct SerializationBasics
    {
        i32                 intValue{};
        String              text{};
        RID                 asset{};
        NestedStruct        nestedStruct{};
        Array<i32>          intArray{};
        Array<RID>          ridArray{};
        Array<NestedStruct> structArray{};
        Array<Array<i32>>   arrayOfArray{};
        Array<i32>          emptyArray{};
    };


    struct SerializationSubObjectBasics
    {

    };

    void Check(const SerializationBasics& a, const SerializationBasics& b)
    {
        CHECK(a.intValue == b.intValue);
        CHECK(a.text == b.text);
        CHECK(a.asset == b.asset);
        CHECK(a.nestedStruct == b.nestedStruct);
        CHECK(a.intArray == b.intArray);
        CHECK(a.ridArray == b.ridArray);
        CHECK(a.structArray == b.structArray);
        CHECK(a.arrayOfArray == b.arrayOfArray);
        CHECK(a.emptyArray == b.emptyArray);
    }

    void RegisterTypes()
    {
        auto nestedStruct = Registry::Type<NestedStruct>();
        nestedStruct.Field<&NestedStruct::value>("value");
        nestedStruct.Field<&NestedStruct::string>("string");

        auto serializationBasics = Registry::Type<SerializationBasics>();
        serializationBasics.Field<&SerializationBasics::intValue>("intValue");
        serializationBasics.Field<&SerializationBasics::text>("text");
        serializationBasics.Field<&SerializationBasics::asset>("asset");
        serializationBasics.Field<&SerializationBasics::nestedStruct>("nestedStruct");
        serializationBasics.Field<&SerializationBasics::intArray>("intArray");
        serializationBasics.Field<&SerializationBasics::ridArray>("ridArray");
        serializationBasics.Field<&SerializationBasics::structArray>("structArray");
        serializationBasics.Field<&SerializationBasics::arrayOfArray>("arrayOfArray");
        serializationBasics.Field<&SerializationBasics::emptyArray>("emptyArray");
        //serializationBasics.Attribute<Resource>();

    }

    TEST_CASE("Resource::SerializationParserObjectBasics")
    {
        Engine::Init();

        RegisterTypes();

        RID rid = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("8658b8ba-0bcd-4e9d-b940-c6f3290ca917"));
        RID rid1 = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("9cbb7cf9-c76c-436f-b6ae-1f31b7eda6b8"));
        RID rid2 = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("8a5021d0-2df9-4a30-9341-bc5c61152c5f"));
        RID rid3 = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("af184ba3-f0ca-48e1-9641-d1f439bae905"));

        SerializationBasics serializationBasics{
            .intValue = 123,
            .text = "MyText",
            .asset = rid,
            .nestedStruct = {
                .value = 7777,
                .string = "Struct.String"
            },
            .intArray = {10, 20, 30},
            .ridArray = {rid1, rid2, rid3},
            .structArray = {
                {
                    .value = 11,
                    .string = "11"
                },
                {
                    .value = 22,
                    .string = "22"
                },
            },
            .arrayOfArray = {
                {1, 2, 3, 4},
                {5, 6, 7, 8}
            },
            .emptyArray = {},
        };


        String str = ResourceSerialization::WriteObject(&serializationBasics, Registry::FindType<SerializationBasics>());

        std::cout << str.CStr() << std::endl;

        CHECK(!str.Empty());

        SerializationBasics serializationBasicsParsed{};
        ResourceSerialization::ParseObject(str, &serializationBasicsParsed, Registry::FindType<SerializationBasics>());

        Check(serializationBasicsParsed, serializationBasics);

        Engine::Destroy();
    }
    
}