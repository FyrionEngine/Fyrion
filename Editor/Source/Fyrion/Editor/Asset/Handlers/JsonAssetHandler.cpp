
#include "JsonAssetHandler.hpp"
#include "Fyrion/Core/Repository.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void JsonAssetHandler::Save(StringView newPath, AssetFile* assetFile)
    {
        {
            String infoFile = Path::Join(newPath, ".info");
            JsonAssetWriter writer;
            ArchiveObject   root = writer.CreateObject();
            writer.WriteString(root, "uuid", assetFile->uuid.ToString());
            FileSystem::SaveFileAsString(infoFile, JsonAssetWriter::Stringify(root));
        }

        {
            VoidPtr      asset = Repository::Load(assetFile->uuid);
            TypeHandler* typeHandler = Repository::GetTypeHandler(assetFile->uuid);

            JsonAssetWriter writer;
            if (ArchiveObject assetArchive = Serialization::Serialize(typeHandler, writer, asset))
            {
                FileSystem::SaveFileAsString(newPath, JsonAssetWriter::Stringify(assetArchive));
            }
        }
    }

    void JsonAssetHandler::Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance)
    {
        JsonAssetReader reader(FileSystem::ReadFileAsString(assetFile->absolutePath));
        Serialization::Deserialize(typeHandler, reader, reader.ReadObject(), instance);
    }
}
