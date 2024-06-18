#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"

using namespace Fyrion;

namespace
{
    struct TestAsset final : Asset
    {
        FY_BASE_TYPES(Asset);

        Subobject subobjects;

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
            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>();
                prototype->subobjects.Add(subobject);
            }
            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>();
                prototype->subobjects.Add(subobject);
            }

            TestAsset* testAsset = AssetDatabase::CreateFromPrototype<TestAsset>(prototype);
            {
                SubobjectAsset* subobject = AssetDatabase::Create<SubobjectAsset>();
                testAsset->subobjects.Add(subobject);
            }

            CHECK(testAsset->GetPrototype() == prototype);
            CHECK(testAsset->subobjects.Count() == 3);

        }
        Engine::Destroy();
    }
}
