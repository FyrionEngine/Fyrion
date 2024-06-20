#include "AssetTypes.hpp"


namespace Fyrion
{
    void AssetIO::RegisterType(NativeTypeHandler<AssetIO>& type)
    {
        type.Function<&AssetIO::GetImportExtensions>("GetImportExtensions");
        type.Function<&AssetIO::ImportAsset>("ImportAsset");
    }

    void AssetDirectory::RegisterType(NativeTypeHandler<AssetDirectory>& type)
    {
        type.Field<&AssetDirectory::children>("children");
    }

    void RegisterAssetTypes()
    {
        Registry::Type<Asset>();
        Registry::Type<AssetDirectory>();
        Registry::Type<AssetIO>();
    }


}
