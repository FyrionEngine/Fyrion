#include "MeshAsset.hpp"

#include "Fyrion/Graphics/MeshUtils.hpp"

namespace Fyrion
{
    void MeshAsset::SetData(Array<VertexStride>&         p_vertices,
                            Array<u32>&                  p_indices,
                            Array<MeshPrimitive>&        p_primitives,
                            const Array<MaterialAsset*>& p_materials,
                            bool                         missingNormals,
                            bool                         missingTangents)
    {

        if (missingNormals)
        {
            //TODO
        }

        if (missingTangents)
        {
            MeshUtils::CalcTangents(p_vertices, p_indices, true);
        }

        indicesCount = p_indices.Size();
        verticesCount = p_vertices.Size();
        primitiveCount = p_primitives.Size();

        boundingBox = MeshUtils::CalculateMeshAABB(p_vertices);

        materials = p_materials;

        SaveBlob(primitives, p_primitives.Data(), p_primitives.Size());
        SaveBlob(vertices, p_vertices.Data(), p_vertices.Size());
        SaveBlob(indices, p_indices.Data(), p_indices.Size());
    }

    void MeshAsset::RegisterType(NativeTypeHandler<MeshAsset>& type)
    {
        type.Field<&MeshAsset::boundingBox>("boundingBox");
        type.Field<&MeshAsset::indicesCount>("indicesCount");
        type.Field<&MeshAsset::verticesCount>("verticesCount");
        type.Field<&MeshAsset::primitiveCount>("primitiveCount");
        type.Field<&MeshAsset::materials>("materials");
        type.Field<&MeshAsset::primitives>("primitives");
        type.Field<&MeshAsset::vertices>("vertices");
        type.Field<&MeshAsset::indices>("indices");
    }
}
