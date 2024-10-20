#include "MeshAssetHandler.hpp"

#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Graphics/RenderUtils.hpp"
#include "Fyrion/IO/FileTypes.hpp"


namespace Fyrion
{
    StringView MeshAssetHandler::Extension()
    {
        return ".mesh";
    }

    TypeID MeshAssetHandler::GetAssetTypeID()
    {
        return GetTypeID<MeshAsset>();
    }

    void MeshAssetHandler::OpenAsset(AssetFile* assetFile) {}

    Image MeshAssetHandler::GenerateThumbnail(AssetFile* assetFile)
    {
        return {};
    }

    void MeshImporter::ImportMesh(MeshImportData& data)
    {
        if (data.missingNormals)
        {
            //TODO
        }

        if (data.missingTangents)
        {
            RenderUtils::CalcTangents(data.vertices, data.indices, true);
        }

        data.meshAsset->indicesCount = data.indices.Size();
        data.meshAsset->verticesCount = data.vertices.Size();
        data.meshAsset->boundingBox = RenderUtils::CalculateMeshAABB(data.vertices);
        data.meshAsset->materials = data.materials;
        data.meshAsset->primitives = data.primitives;

        OutputFileStream stream = data.assetFile->CreateStream();
        stream.Write(reinterpret_cast<u8*>(data.vertices.Data()), data.vertices.Size() * sizeof(VertexStride));
        stream.Write(reinterpret_cast<u8*>(data.indices.Data()), data.indices.Size() * sizeof(u32));
        stream.Close();
    }
}
