#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{

    struct TxtAsset : Asset
    {
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
            return nullptr;
        }
    };


    void RegisterTypes()
    {
        Registry::Type<TxtAsset>();
        Registry::Type<TxtAssetIO>();
    }

    struct AssetDirectory;
    TEST_CASE("Asset::AssetDirectory")
    {

        Engine::Init();
        RegisterTypes();

        {
            String assetPath = Path::Join(FY_TEST_FILES, "AssetsBasic");

            AssetDirectory* directory = AssetDatabase::LoadFromDirectory("Fyrion", assetPath);
            REQUIRE(directory);
            CHECK(directory->GetPath() == "Fyrion:/");
        }
        Engine::Destroy();
    }
}
