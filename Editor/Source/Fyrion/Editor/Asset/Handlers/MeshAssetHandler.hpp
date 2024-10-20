#pragma once
#include "JsonAssetHandler.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Graphics/Assets/MeshAsset.hpp"


namespace Fyrion
{
    struct MeshAssetHandler : JsonAssetHandler
    {
        FY_BASE_TYPES(JsonAssetHandler);

        StringView Extension() override;
        TypeID     GetAssetTypeID() override;
        void       OpenAsset(AssetFile* assetFile) override;
        Image      GenerateThumbnail(AssetFile* assetFile) override;
    };


    struct MeshImportData
    {
        AssetFile*                  assetFile;
        MeshAsset*                  meshAsset;
        Array<VertexStride>         vertices;
        Array<u32>                  indices;
        Array<MeshPrimitive>        primitives;
        const Array<MaterialAsset*> materials;
        bool                        missingNormals;
        bool                        missingTangents;
    };


    namespace MeshImporter
    {
        void FY_API ImportMesh(MeshImportData& data);
    }
}
