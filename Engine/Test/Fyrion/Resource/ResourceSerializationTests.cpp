
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

    struct SerializationResourceBasics
    {
        constexpr static u32 StringValue           = 0;
        constexpr static u32 AssetValue            = 1;
        constexpr static u32 StructValue           = 2;
        constexpr static u32 RIDArray              = 3;
        constexpr static u32 Text                  = 4;
        constexpr static u32 Subobject             = 5;
        constexpr static u32 PrototypeNoOverride   = 6;
        constexpr static u32 PrototypeWithOverride = 7;
        constexpr static u32 SubobjectSet          = 8;
    };

    struct SerializationSubObjectBasics
    {
        constexpr static u32  StringValue = 0;
        constexpr static u32  IntValue = 1;
    };

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

        ResourceTypeBuilder<SerializationResourceBasics>::Builder()
            .Value<SerializationResourceBasics::StringValue, String>("StringValue")
            .Value<SerializationResourceBasics::AssetValue, RID>("AssetValue")
            .Value<SerializationResourceBasics::StructValue, NestedStruct>("StructValue")
            .Value<SerializationResourceBasics::RIDArray, Array<RID>>("RIDArray")
            .Value<SerializationResourceBasics::Text, String>("Text")
            .SubObject<SerializationResourceBasics::Subobject>("Subobject")
            .SubObject<SerializationResourceBasics::PrototypeNoOverride>("PrototypeNoOverride")
            .SubObject<SerializationResourceBasics::PrototypeWithOverride>("PrototypeWithOverride")
            .SubObjectSet<SerializationResourceBasics::SubobjectSet>("SubobjectSet")
            .Build();

        ResourceTypeBuilder<SerializationSubObjectBasics>::Builder()
            .Value<SerializationSubObjectBasics::StringValue, String>("StringValue")
            .Value<SerializationSubObjectBasics::IntValue, i32>("IntValue")
            .Build();
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
                .string = "Struct.string"
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

        CHECK(!str.Empty());

        SerializationBasics serializationBasicsParsed{};
        ResourceSerialization::ParseObject(str, &serializationBasicsParsed, Registry::FindType<SerializationBasics>());

        Check(serializationBasicsParsed, serializationBasics);

        Engine::Destroy();
    }


    TEST_CASE("Core::SerializationWriterResourceBasics")
    {
        NestedStruct nestedStruct{
            .value = 11,
            .string = "Nested",
        };

        String str;
        String prototypeStr;
        String prototype2Str;
        {
            Engine::Init();
            RegisterTypes();
            RID rid;

            {
                rid = Repository::CreateResource<SerializationResourceBasics>();
                RID ridAsset = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("8658b8ba-0bcd-4e9d-b940-c6f3290ca917"));

                RID rid1 = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("9cbb7cf9-c76c-436f-b6ae-1f31b7eda6b8"));
                RID rid2 = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("8a5021d0-2df9-4a30-9341-bc5c61152c5f"));
                RID rid3 = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("af184ba3-f0ca-48e1-9641-d1f439bae905"));

                Array<RID> rids{
                    rid1, rid2, rid3
                };

                RID ridSubObject = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("ffc207a1-1db6-4bfe-b1f1-c601770b4ee2"));
                {
                    ResourceObject write =  Repository::Write(ridSubObject);
                    write.SetValue(SerializationSubObjectBasics::StringValue, String{"TestSubObject"});
                    write.SetValue(SerializationSubObjectBasics::IntValue, 445566);
                    write.Commit();
                }

                RID prototype = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("4618f8b1-97ee-4402-99f9-d4b5c9ce789d"));
                RID prototypeNoOverride = Repository::CreateFromPrototype(prototype, UUID::FromString("c5a8671c-6803-4e05-803c-9b42d9d8b62f"));
                RID prototypeWithOverride = Repository::CreateFromPrototype(prototype, UUID::FromString("d9a0cd3c-1be7-4f54-bbdd-4213bc4a15a1"));
                {
                    ResourceObject object = Repository::Write(prototypeWithOverride);
                    object.SetValue(SerializationSubObjectBasics::StringValue, String{"OverrideValue"});
                    object.Commit();
                }


                ResourceObject write = Repository::Write(rid);
                write.SetValue(SerializationResourceBasics::StringValue, String{"blahblah"});
                write.SetValue(SerializationResourceBasics::AssetValue, ridAsset);
                write.SetValue(SerializationResourceBasics::StructValue, nestedStruct);
                write.SetValue(SerializationResourceBasics::RIDArray, rids);
                //		    write.SetValue(SerializationResourceBasics::Text, );
                write.SetSubObject(SerializationResourceBasics::Subobject, ridSubObject);
                write.SetSubObject(SerializationResourceBasics::PrototypeNoOverride, prototypeNoOverride);
                write.SetSubObject(SerializationResourceBasics::PrototypeWithOverride, prototypeWithOverride);


                {
                    RID subobject = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("ebb81da8-04f5-4b8f-ba52-6b5e45473c2c"));
                    ResourceObject writeSubObject = Repository::Write(subobject);
                    writeSubObject.SetValue(SerializationSubObjectBasics::StringValue, String{"SubobjectSetString1"});
                    writeSubObject.SetValue(SerializationSubObjectBasics::IntValue, 111);
                    writeSubObject.Commit();

                    write.AddToSubObjectSet(SerializationResourceBasics::SubobjectSet, subobject);
                }

                {
                    RID subobject = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("fe9e91c6-c629-4837-86b8-d78efce286dc"));
                    ResourceObject writeSubObject = Repository::Write(subobject);
                    writeSubObject.SetValue(SerializationSubObjectBasics::StringValue, String{"SubobjectSetString2"});
                    writeSubObject.SetValue(SerializationSubObjectBasics::IntValue, 222);
                    writeSubObject.Commit();

                    write.AddToSubObjectSet(SerializationResourceBasics::SubobjectSet, subobject);
                }

                {
                    RID subobject = Repository::CreateResource<NestedStruct>(UUID::FromString("e1655559-7c1e-4afb-a931-8257694705f1"));

                    NestedStruct nestedStruct = NestedStruct{
                        .value = 123,
                        .string = "Str",
                    };
                    Repository::Commit(subobject, &nestedStruct);

                    write.AddToSubObjectSet(SerializationResourceBasics::SubobjectSet, subobject);

                }

                write.Commit();
                str = ResourceSerialization::WriteResource(rid);
                CHECK(!str.Empty());
            }

            {
                //Create prototype after the serialization to make sure it will work even if it's create after the serialization
                RID prototype = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("4618f8b1-97ee-4402-99f9-d4b5c9ce789d"));
                ResourceObject write =  Repository::Write(prototype);
                write.SetValue(SerializationSubObjectBasics::StringValue, String{"TestStrPrototype"});
                write.SetValue(SerializationSubObjectBasics::IntValue, 667788);
                write.Commit();

                prototypeStr = ResourceSerialization::WriteResource(prototype);
                CHECK(!prototypeStr.Empty());

            }


            {
                RID prototype = Repository::CreateFromPrototype(rid);
                ResourceObject write =  Repository::Write(prototype);
                {
                    RID subobjectItem = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("82e8aab4-7981-4c83-9b3f-c4d0738c4337"));
                    ResourceObject subObjectResource = Repository::Write(subobjectItem);
                    subObjectResource.SetValue(SerializationSubObjectBasics::StringValue, String{"SubobjectSetString3"});
                    subObjectResource.SetValue(SerializationSubObjectBasics::IntValue, 333);
                    subObjectResource.Commit();
                    write.AddToSubObjectSet(SerializationResourceBasics::SubobjectSet, subobjectItem);
                }

                write.RemoveFromPrototypeSubObjectSet(SerializationResourceBasics::SubobjectSet, Repository::GetByUUID(UUID::FromString("ebb81da8-04f5-4b8f-ba52-6b5e45473c2c")));
                write.Commit();

                prototype2Str = ResourceSerialization::WriteResource(prototype);

            }
            Engine::Destroy();
        }

        {
            Engine::Init();
            RegisterTypes();

            {
                RID ridPrototype = Repository::CreateResource<SerializationSubObjectBasics>(UUID::FromString("4618f8b1-97ee-4402-99f9-d4b5c9ce789d"));
                ResourceSerialization::ParseResource(prototypeStr, ridPrototype);

                ResourceObject prototype = Repository::Read(ridPrototype);
                CHECK(prototype.GetValue<String>(SerializationSubObjectBasics::StringValue) == String{"TestStrPrototype"});
                CHECK(prototype.GetValue<i32>(SerializationSubObjectBasics::IntValue) == 667788);
            }


            RID rid = Repository::CreateResource<SerializationResourceBasics>();
            ResourceSerialization::ParseResource(str, rid);

            ResourceObject read = Repository::Read(rid);
            CHECK(read.GetValue<String>(SerializationResourceBasics::StringValue) == "blahblah");
            RID assetRid = read.GetValue<RID>(SerializationResourceBasics::AssetValue);
            CHECK(assetRid);
            CHECK(Repository::GetUUID(assetRid) == UUID::FromString("8658b8ba-0bcd-4e9d-b940-c6f3290ca917"));

            CHECK(nestedStruct == read.GetValue<NestedStruct>(SerializationResourceBasics::StructValue));

            const Array<RID>& rids = read.GetValue<Array<RID>>(SerializationResourceBasics::RIDArray);
            CHECK(rids.Size() == 3);

            CHECK(Repository::GetUUID(rids[0]) == UUID::FromString("9cbb7cf9-c76c-436f-b6ae-1f31b7eda6b8"));
            CHECK(Repository::GetUUID(rids[1]) == UUID::FromString("8a5021d0-2df9-4a30-9341-bc5c61152c5f"));
            CHECK(Repository::GetUUID(rids[2]) == UUID::FromString("af184ba3-f0ca-48e1-9641-d1f439bae905"));

            {
                RID ridSubobject = read.GetSubObject(SerializationResourceBasics::Subobject);
                CHECK(ridSubobject);

                ResourceObject subobject = Repository::Read(ridSubobject);
                CHECK(subobject.GetValue<String>(SerializationSubObjectBasics::StringValue) == String{"TestSubObject"});
                CHECK(subobject.GetValue<i32>(SerializationSubObjectBasics::IntValue) == 445566);
            }

            u32 subobjectSetCount = read.GetSubObjectSetCount(SerializationResourceBasics::SubobjectSet);
            Array<RID> subobjectSet(subobjectSetCount);
            read.GetSubObjectSet(SerializationResourceBasics::SubobjectSet, subobjectSet);

            u32 count = 0;
            for(const RID subobjectRid: subobjectSet)
            {
                if (Repository::GetUUID(subobjectRid) == UUID::FromString("ebb81da8-04f5-4b8f-ba52-6b5e45473c2c"))
                {
                    ResourceObject subobject = Repository::Read(subobjectRid);
                    CHECK(subobject.GetValue<String>(SerializationSubObjectBasics::StringValue) == String{"SubobjectSetString1"});
                    CHECK(subobject.GetValue<i32>(SerializationSubObjectBasics::IntValue) == 111);
                    count++;
                }
                else if (Repository::GetUUID(subobjectRid) == UUID::FromString("fe9e91c6-c629-4837-86b8-d78efce286dc"))
                {
                    ResourceObject subobject = Repository::Read(subobjectRid);
                    CHECK(subobject.GetValue<String>(SerializationSubObjectBasics::StringValue) == String{"SubobjectSetString2"});
                    CHECK(subobject.GetValue<i32>(SerializationSubObjectBasics::IntValue) == 222);
                    count++;
                }
                else if (Repository::GetUUID(subobjectRid) == UUID::FromString("e1655559-7c1e-4afb-a931-8257694705f1"))
                {
                    const NestedStruct& nestedStruct = Repository::ReadData<NestedStruct>(subobjectRid);
                    CHECK(nestedStruct.string == "Str");
                    CHECK(nestedStruct.value == 123);
                    count++;
                }
            }
            CHECK(subobjectSetCount == count);

            RID ridPrototypeNoOverride = read.GetSubObject(SerializationResourceBasics::PrototypeNoOverride);
            CHECK(ridPrototypeNoOverride);
            ResourceObject prototypeNoOverride =Repository::Read(ridPrototypeNoOverride);
            CHECK(prototypeNoOverride.GetValue<String>(SerializationSubObjectBasics::StringValue) == String{"TestStrPrototype"});
            CHECK(prototypeNoOverride.GetValue<i32>(SerializationSubObjectBasics::IntValue) == 667788);


            RID ridPrototypeWithOverride = read.GetSubObject(SerializationResourceBasics::PrototypeWithOverride);
            CHECK(ridPrototypeWithOverride);
            ResourceObject prototypeWithOverride =Repository::Read(ridPrototypeWithOverride);
            CHECK(prototypeWithOverride.GetValue<String>(SerializationSubObjectBasics::StringValue) == String{"OverrideValue"});
            CHECK(prototypeWithOverride.GetValue<i32>(SerializationSubObjectBasics::IntValue) == 667788);

            RID ridPrototypeAll =  Repository::CreateFromPrototype(rid);
            ResourceSerialization::ParseResource(prototype2Str, ridPrototypeAll);

            {
                ResourceObject subobject = Repository::Read(ridPrototypeAll);
                const NestedStruct& value = read.GetValue<NestedStruct>(SerializationResourceBasics::StructValue);
                const NestedStruct& valueSubObject = subobject.GetValue<NestedStruct>(SerializationResourceBasics::StructValue);
                CHECK(value.value == valueSubObject.value);
                CHECK(value.string == valueSubObject.string);

                {
                    u32 countSubObjects = subobject.GetSubObjectSetCount(SerializationResourceBasics::SubobjectSet);
                    Array<RID> subObjects(countSubObjects);
                    subobject.GetSubObjectSet(SerializationResourceBasics::SubobjectSet, subObjects);
                    CHECK(subObjects.Size() == 3);
                    u32 checks = 0;
                    for (int i = 0; i < countSubObjects; ++i)
                    {
                        RID subobjectItem = subObjects[i];
                        if (Repository::GetUUID(subobjectItem) == UUID::FromString("82e8aab4-7981-4c83-9b3f-c4d0738c4337"))
                        {
                            ResourceObject subObjectResource = Repository::Read(subobjectItem);
                            CHECK(subObjectResource.GetValue<String>(SerializationSubObjectBasics::StringValue) == "SubobjectSetString3");
                            CHECK(subObjectResource.GetValue<i32>(SerializationSubObjectBasics::IntValue) == 333);
                            checks++;
                        }
                        else if (Repository::GetUUID(subobjectItem) == UUID::FromString("fe9e91c6-c629-4837-86b8-d78efce286dc"))
                        {
                            ResourceObject subObjectResource = Repository::Read(subobjectItem);
                            CHECK(subObjectResource.GetValue<String>(SerializationSubObjectBasics::StringValue) == "SubobjectSetString2");
                            CHECK(subObjectResource.GetValue<i32>(SerializationSubObjectBasics::IntValue) == 222);
                            checks++;
                        }
                        else if (Repository::GetUUID(subobjectItem) == UUID::FromString("e1655559-7c1e-4afb-a931-8257694705f1"))
                        {
                            const NestedStruct& nestedStruct = Repository::ReadData<NestedStruct>(subobjectItem);
                            CHECK(nestedStruct.string == "Str");
                            CHECK(nestedStruct.value == 123);
                            checks++;
                        }
                    }
                    CHECK(checks == countSubObjects);
                }
            }
            Engine::Destroy();
        }
    }

    
}