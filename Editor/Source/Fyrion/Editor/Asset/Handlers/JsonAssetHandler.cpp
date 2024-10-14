
#include "JsonAssetHandler.hpp"
#include "Fyrion/Core/Repository.hpp"
#include "Fyrion/Core/StreamBuffer.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void JsonAssetHandler::Save(StringView newPath, AssetFile* assetFile)
    {
        VoidPtr      asset = Repository::Load(assetFile->uuid);
        TypeHandler* typeHandler = Repository::GetTypeHandler(assetFile->uuid);

        OutputFileStream stream(Path::Join(newPath, ".buffer"));
        JsonAssetWriter writer(SerializationOptions::None, &stream);

        ArchiveObject assetArchive = Serialization::Serialize(typeHandler, writer, asset);
        FileSystem::SaveFileAsString(newPath, JsonAssetWriter::Stringify(assetArchive));

        stream.Close();
    }

    void JsonAssetHandler::Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance)
    {
        JsonAssetReader reader(FileSystem::ReadFileAsString(assetFile->absolutePath));
        Serialization::Deserialize(typeHandler, reader, reader.ReadObject(), instance);
    }
}
