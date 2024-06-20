#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

using namespace Fyrion;

namespace
{
    struct SubobjectAsset : Asset
    {
        FY_BASE_TYPES(Asset);

        i32 value{};

        static void RegisterType(NativeTypeHandler<SubobjectAsset>& type)
        {
            type.Field<&SubobjectAsset::value>("value");
        }
    };

    struct TestAsset final : Asset
    {
        FY_BASE_TYPES(Asset);

        Subobject<SubobjectAsset>  subobjects;
        Value<i32> testValue;

        static void RegisterType(NativeTypeHandler<TestAsset>& type)
        {
            type.Field<&TestAsset::subobjects>("subobjects");
            type.Field<&TestAsset::testValue>("testValue");
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
                subobject->value = 1;
                prototype->subobjects.Add(subobject);
            }
            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>();
                subobject->value = 2;
                prototype->subobjects.Add(subobject);
            }

            UUID subobjectId = UUID::FromString("939fb3ac-c162-4d5f-b95e-b38e43c3ba69");

            TestAsset* testAsset = AssetDatabase::CreateFromPrototype<TestAsset>(prototype);
            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>(subobjectId);
                subobject->value = 3;
                testAsset->subobjects.Add(subobject);
            }

            CHECK(testAsset->testValue == 10);

            CHECK(testAsset->GetPrototype() == prototype);
            CHECK(testAsset->subobjects.Count() == 3);

            testAsset->subobjects.Remove(AssetDatabase::FindById<SubobjectAsset>(subobjectId));

            CHECK(testAsset->subobjects.Count() == 2);
        }
        Engine::Destroy();
    }

    TEST_CASE("Asset::SubobjectDestroy")
    {
        Engine::Init();
        {
            Registry::Type<TestAsset>();
            Registry::Type<SubobjectAsset>();

            TestAsset* asset = AssetDatabase::Create<TestAsset>();
            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>(UUID::FromString("939fb3ac-c162-4d5f-b95e-b38e43c3ba69"));
                subobject->value = 1;
                asset->subobjects.Add(subobject);
            }

            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>(UUID::FromString("939fb3ac-c162-4d5f-b95e-b38e43c3ba870"));
                subobject->value = 2;
                asset->subobjects.Add(subobject);
            }

            CHECK(AssetDatabase::FindById(UUID::FromString("939fb3ac-c162-4d5f-b95e-b38e43c3ba69")) != nullptr);
            CHECK(AssetDatabase::FindById(UUID::FromString("939fb3ac-c162-4d5f-b95e-b38e43c3ba870")) != nullptr);

            AssetDatabase::Destroy(asset);

            CHECK(AssetDatabase::FindById(UUID::FromString("939fb3ac-c162-4d5f-b95e-b38e43c3ba69")) == nullptr);
            CHECK(AssetDatabase::FindById(UUID::FromString("939fb3ac-c162-4d5f-b95e-b38e43c3ba870")) == nullptr);
        }
        Engine::Destroy();
    }



}
