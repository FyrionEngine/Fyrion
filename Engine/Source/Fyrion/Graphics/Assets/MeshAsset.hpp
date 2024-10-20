#pragma once

#include "MaterialAsset.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/IO/Asset.hpp"

namespace Fyrion
{
    class FY_API MeshAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        static void RegisterType(NativeTypeHandler<MeshAsset>& type);

        Span<MeshPrimitive>  GetPrimitives() const;
        Span<MaterialAsset*> GetMaterials() const;

        AABB                  boundingBox;
        u32                   indicesCount = 0;
        usize                 verticesCount = 0;
        Array<MaterialAsset*> materials;
        Array<MeshPrimitive>  primitives{};
    };
}
