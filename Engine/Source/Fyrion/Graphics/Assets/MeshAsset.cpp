#include "MeshAsset.hpp"

namespace Fyrion
{
    void MeshAsset::RegisterType(NativeTypeHandler<MeshAsset>& type)
    {
        type.Field<&MeshAsset::boundingBox>("boundingBox");
        type.Field<&MeshAsset::indicesCount>("indicesCount");
        type.Field<&MeshAsset::verticesCount>("verticesCount");
        type.Field<&MeshAsset::materials>("materials");
        type.Field<&MeshAsset::primitives>("primitives");
        type.Field<&MeshAsset::vertices>("vertices");
        type.Field<&MeshAsset::indices>("indices");
    }
}
