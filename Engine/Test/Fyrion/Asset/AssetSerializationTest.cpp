#include "doctest.h"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetSerialization.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"

using namespace Fyrion;

namespace
{
    struct TestSubObject
    {
        u32 otherValue;

        static void RegisterType(NativeTypeHandler<TestSubObject>& type)
        {
            type.Field<&TestSubObject::otherValue>("otherValue");
        }
    };

    struct TestSerialization
    {
        u32           value;
        String        string;
        f32           floatValue;
        TestSubObject subObject;

        static void RegisterType(NativeTypeHandler<TestSerialization>& type)
        {
            type.Field<&TestSerialization::value>("value");
            type.Field<&TestSerialization::string>("string");
            type.Field<&TestSerialization::floatValue>("floatValue");
            type.Field<&TestSerialization::subObject>("subObject");
        }
    };

    TEST_CASE("AssetSerialization::JsonWriterReaderBasics")
    {
        JsonAssetWriter writer{};

        ArchiveObject root = writer.CreateObject();

        writer.WriteInt(root, "testInt", 10);
        writer.WriteString(root, "testStr", "strstr");
        writer.WriteUUID(root, "testUUID", UUID::RandomUUID());

        {
            ArchiveObject subObject = writer.CreateObject();
            writer.WriteString(subObject, "strSubObject", "strstr");
            writer.WriteValue(root, "subobj", subObject);
        }

        {
            ArchiveObject arr = writer.CreateArray();
            writer.AddInt(arr, 10);
            writer.AddInt(arr, 30);
            writer.AddUUID(arr, UUID::RandomUUID());
            writer.WriteValue(root, "arr", arr);
        }
    }


    TEST_CASE("AssetSerialization::JsonTypeSerialization")
    {
        Engine::Init();
        {
            {
                Registry::Type<TestSerialization>();
                Registry::Type<TestSubObject>();
            }

            {
                TypeHandler* typeHandler = Registry::FindType<TestSerialization>();

                TestSerialization vl = {
                    .value = 47,
                    .string = "testring",
                    .floatValue = 44.475,
                    .subObject = {
                        .otherValue = 444
                    }
                };

                JsonAssetWriter writer{};
                ArchiveObject   obj = typeHandler->Serialize(writer, &vl);

                String str = JsonAssetWriter::Stringify(obj);
                CHECK(!str.Empty());
                MESSAGE(doctest::String(str.CStr()));
            }
        }

        Engine::Destroy();
    }
}
