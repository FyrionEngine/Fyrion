#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Editor/Asset/AssetImporter.hpp"

namespace Fyrion
{
    struct TextureIO final : AssetImporter
    {
        FY_BASE_TYPES(AssetImporter);

        Array<String> ImportExtensions() override
        {
            return {".png", ".jpg", ".jpeg", ".tga", "bmp", ".hdr"};
        }

        TypeID GetImportSettings() override
        {
            return 0;
        }

        bool ImportAsset(AssetEditor& assetEditor, StringView path, ConstPtr importSettings) override
        {
            //assetEditor.CreateAsset()


            return false;
        }
    };


    void RegisterTextureIO()
    {
        Registry::Type<TextureIO>();
    }
}
