#include "MeshAsset.hpp"

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/RenderUtils.hpp"

namespace Fyrion
{

    Span<MeshPrimitive> MeshAsset::GetPrimitives() const
    {
        return primitives;
    }

    Span<MaterialAsset*> MeshAsset::GetMaterials() const
    {
        return materials;
    }


    void MeshAsset::RegisterType(NativeTypeHandler<MeshAsset>& type)
    {
        type.Field<&MeshAsset::boundingBox>("boundingBox");
        type.Field<&MeshAsset::indicesCount>("indicesCount");
        type.Field<&MeshAsset::verticesCount>("verticesCount");
        type.Field<&MeshAsset::materials>("materials");
        type.Field<&MeshAsset::primitives>("primitives");

        // type.Field<&MeshAsset::vertices>("vertices");
        // type.Field<&MeshAsset::indices>("indices");
    }
}
