#pragma once
#include "MaterialAsset.hpp"
#include "Fyrion/Asset/Asset.hpp"

namespace Fyrion
{
    class FY_API MeshAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        static void RegisterType(NativeTypeHandler<MeshAsset>& type);

    private:
        AABB                  boundingBox;
        u32                   indicesCount = 0;
        usize                 verticesCount = 0;
        Array<MaterialAsset*> materials;
        Blob                  primitives{};
        Blob                  vertices{};
        Blob                  indices{};
    };
}
