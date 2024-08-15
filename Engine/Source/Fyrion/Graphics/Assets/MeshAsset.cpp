#include "MeshAsset.hpp"

#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/RenderUtils.hpp"

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
            RenderUtils::CalcTangents(p_vertices, p_indices, true);
        }

        indicesCount = p_indices.Size();
        verticesCount = p_vertices.Size();

        boundingBox = RenderUtils::CalculateMeshAABB(p_vertices);

        materials = p_materials;
        primitives = p_primitives;

        SaveBuffer(vertices, p_vertices.Data(), p_vertices.Size() * sizeof(VertexStride));
        SaveBuffer(indices, p_indices.Data(), p_indices.Size() * sizeof(u32));
    }

    Span<MeshPrimitive> MeshAsset::GetPrimitives() const
    {
        return primitives;
    }

    Buffer MeshAsset::GetVertexBuffer()
    {
        if (!vertexBuffer)
        {
            Array<u8> data = LoadBuffer(vertices);

            BufferCreation creation{
                .usage = BufferUsage::VertexBuffer,
                .size = data.Size(),
                .allocation = BufferAllocation::GPUOnly
            };

            vertexBuffer = Graphics::CreateBuffer(creation);

            Graphics::UpdateBufferData(BufferDataInfo{
                .buffer = vertexBuffer,
                .data = data.Data(),
                .size = data.Size(),
            });
        }
        return vertexBuffer;
    }

    Buffer MeshAsset::GetIndexBuffeer()
    {
        if (!indexBuffer)
        {
            Array<u8> data = LoadBuffer(indices);

            BufferCreation creation{
                .usage = BufferUsage::IndexBuffer,
                .size = data.Size(),
                .allocation = BufferAllocation::GPUOnly
            };

            indexBuffer = Graphics::CreateBuffer(creation);

            Graphics::UpdateBufferData(BufferDataInfo{
                .buffer = indexBuffer,
                .data = data.Data(),
                .size = data.Size(),
            });
        }

        return indexBuffer;
    }

    Span<MaterialAsset*> MeshAsset::GetMaterials() const
    {
        return materials;
    }

    MeshAsset::~MeshAsset()
    {
        if (vertexBuffer)
        {
            Graphics::DestroyBuffer(vertexBuffer);
        }

        if (indexBuffer)
        {
            Graphics::DestroyBuffer(indexBuffer);
        }
    }

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
