#pragma once
#include "MaterialAsset.hpp"
#include "Fyrion/Asset/Asset.hpp"

namespace Fyrion
{
    class FY_API MeshAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        ~MeshAsset() override;

        static void RegisterType(NativeTypeHandler<MeshAsset>& type);

        void SetData(Array<VertexStride>&         p_vertices,
                     Array<u32>&                  p_indices,
                     Array<MeshPrimitive>&        p_primitives,
                     const Array<MaterialAsset*>& p_materials,
                     bool                         missingNormals,
                     bool                         missingTangents);

        Span<MeshPrimitive>  GetPrimitives() const;
        Span<MaterialAsset*> GetMaterials() const;

        Buffer GetVertexBuffer();
        Buffer GetIndexBuffeer();

    private:
        AABB                  boundingBox;
        u32                   indicesCount = 0;
        usize                 verticesCount = 0;
        Array<MaterialAsset*> materials;
        Array<MeshPrimitive>  primitives{};
        AssetBuffer                  vertices{};
        AssetBuffer                  indices{};


        Buffer vertexBuffer{};
        Buffer indexBuffer{};
    };
}
