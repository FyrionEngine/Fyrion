#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"

using namespace Fyrion;

namespace
{
    struct TestAsset final : Asset
    {
        FY_BASE_TYPES(Asset);

        Subobject  subobjects;
        Value<i32> testValue;

        static void RegisterType(NativeTypeHandler<TestAsset>& type)
        {
            type.Field<&TestAsset::subobjects>("subobjects");
        }
    };

    struct SubobjectAsset : Asset
    {
        FY_BASE_TYPES(Asset);

        i32 value;

        static void RegisterType(NativeTypeHandler<SubobjectAsset>& type)
        {
            type.Field<&SubobjectAsset::value>("value");
        }
    };


    TEST_CASE("Asset::Basic")
    {
        Engine::Init();
        {
            Registry::Type<TestAsset>();

            AssetDirectory* root = AssetDatabase::Create<AssetDirectory>();
            REQUIRE(root);

            AssetDirectory* anoterDir = AssetDatabase::Create<AssetDirectory>();
            root->children.Add(anoterDir);

            TestAsset* testAsset = AssetDatabase::Create<TestAsset>();
            root->children.Add(testAsset);

            Array<Asset*> children = root->children.GetAsArray();
            CHECK(children.Size() == 2);
        }
        Engine::Destroy();
    }

    TEST_CASE("Asset::Prototypes")
    {
        Engine::Init();
        {
            Registry::Type<TestAsset>();
            Registry::Type<SubobjectAsset>();

            TestAsset* prototype = AssetDatabase::Create<TestAsset>();
            CHECK(!prototype->testValue);
            prototype->testValue = 10;
            CHECK(prototype->testValue);
            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>();
                prototype->subobjects.Add(subobject);
            }
            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>();
                prototype->subobjects.Add(subobject);
            }

            UUID subobjectId = UUID::FromString("939fb3ac-c162-4d5f-b95e-b38e43c3ba69");

            TestAsset* testAsset = AssetDatabase::CreateFromPrototype<TestAsset>(prototype);
            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>(subobjectId);
                testAsset->subobjects.Add(subobject);
            }

            //CHECK(testAsset->testValue == 10);

            CHECK(testAsset->GetPrototype() == prototype);
            CHECK(testAsset->subobjects.Count() == 3);

            testAsset->subobjects.Remove(AssetDatabase::FindById(subobjectId));

            CHECK(testAsset->subobjects.Count() == 2);
        }
        Engine::Destroy();
    }
}
