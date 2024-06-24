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
        String str;

        {
            JsonAssetWriter writer;

            ArchiveObject root = writer.CreateObject();

            writer.WriteInt(root, "testInt", 10);
            writer.WriteString(root, "testStr", "strstr");

            {
                ArchiveObject subObject = writer.CreateObject();
                writer.WriteString(subObject, "strSubObject", "strstr");
                writer.WriteValue(root, "subobj", subObject);
            }

            {
                ArchiveObject arr = writer.CreateArray();
                writer.AddInt(arr, 10);
                writer.AddInt(arr, 30);
                writer.WriteValue(root, "arr", arr);
            }

            {
                ArchiveObject objAray = writer.CreateArray();

                for (int i = 0; i < 5; ++i)
                {
                    ArchiveObject obj = writer.CreateObject();
                    writer.WriteInt(obj, "vl", i);
                    writer.AddValue(objAray, obj);

                }

                writer.WriteValue(root, "objAray", objAray);
            }

            str = JsonAssetWriter::Stringify(root);
        }

        CHECK(!str.Empty());

        {
            JsonAssetReader reader(str);
            ArchiveObject root = reader.ReadObject();
            CHECK(reader.ReadInt(root, "testInt") == 10);
            CHECK(reader.ReadString(root, "testStr") == "strstr");

            ArchiveObject arr = reader.ReadObject(root, "arr");

            Array<i32> toArr{};
            toArr.Resize(reader.ArrSize(arr));

            ArchiveObject item{};
            for (usize i = 0; i < reader.ArrSize(arr); ++i)
            {
                item = reader.Next(arr, item);
                toArr[i] = static_cast<i32>(reader.GetInt(item));
            }

            REQUIRE(toArr.Size() == 2);
            CHECK(toArr[0] == 10);
            CHECK(toArr[1] == 30);
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

                String str;
                {
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

                    str = JsonAssetWriter::Stringify(obj);
                }

                CHECK(!str.Empty());

                {
                    TestSerialization vl{};
                    JsonAssetReader reader(str);
                    typeHandler->Deserialize(reader, reader.ReadObject(), &vl);

                    CHECK(vl.value == 47);
                    CHECK(vl.string == "testring");
                    CHECK(vl.subObject.otherValue == 444);
                }

            }
        }

        Engine::Destroy();
    }
}
