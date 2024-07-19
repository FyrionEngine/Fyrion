#include "DCCAsset.hpp"

namespace Fyrion
{
    void DCCAsset::RegisterType(NativeTypeHandler<DCCAsset>& type)
    {
        type.Field<&DCCAsset::scaleFactor>("scaleFactor");
        type.Field<&DCCAsset::mergeMaterials>("mergeMaterials");
        type.Field<&DCCAsset::mergeMeshes>("mergeMeshes");
    }
}
