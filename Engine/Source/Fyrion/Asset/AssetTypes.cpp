#include "AssetTypes.hpp"


namespace Fyrion
{

    void AssetDirectory::RegisterType(NativeTypeHandler<AssetDirectory>& type)
    {
        type.Field<&AssetDirectory::children>("children");
    }

    void RegisterAssetTypes()
    {
        Registry::Type<AssetField>();
        Registry::Type<Subobject>();
        Registry::Type<Asset>();
        Registry::Type<AssetDirectory>();
    }


}
