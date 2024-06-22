#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{

    struct TxtAsset : Asset
    {
        FY_BASE_TYPES(Asset);

        Value<String> text;

        static void RegisterType(NativeTypeHandler<TxtAsset>& type)
        {
            type.Field<&TxtAsset::text>("text");
        }
    };

    struct TxtAssetIO : AssetIO
    {
        FY_BASE_TYPES(AssetIO);

        StringView extension = {".txt"};

        Span<StringView> GetImportExtensions() override
        {
            return {&extension};
        }

        Asset* ImportAsset(StringView path, Asset* reimportAsset) override
        {
            TxtAsset* txtAsset = AssetDatabase::Create<TxtAsset>();
            txtAsset->text = FileSystem::ReadFileAsString(path);
            return txtAsset;
        }
    };


    void RegisterTypes()
    {
        Registry::Type<TxtAsset>();
        Registry::Type<TxtAssetIO>();
    }

    TEST_CASE("Asset::LoadFromDirectory")
    {

        Engine::Init();
        RegisterTypes();

        {
            String assetPath = Path::Join(FY_TEST_FILES, "AssetsBasic");

            AssetDirectory* directory = AssetDatabase::LoadFromDirectory("Fyrion", assetPath);
            REQUIRE(directory);
            CHECK(directory->GetPath() == "Fyrion:/");

            TxtAsset* txtAsset = AssetDatabase::FindByPath<TxtAsset>("Fyrion://Dir1/Dir1/TxtFile1.txt");
            REQUIRE(txtAsset);

            CHECK(txtAsset->text == "aaaa");

        }
        Engine::Destroy();
    }
}
