#include "JsonAssetHandler.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void JsonAssetHandler::Save(StringView newPath, AssetFile* assetFile)
    {
        JsonAssetWriter writer;
        ArchiveObject   assetArchive = Serialization::Serialize(Registry::FindTypeById(GetAssetTypeID()), writer, Assets::Load(assetFile->uuid));
        FileSystem::SaveFileAsString(newPath, JsonAssetWriter::Stringify(assetArchive));
    }

    void JsonAssetHandler::Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance)
    {
        if (String str = FileSystem::ReadFileAsString(assetFile->absolutePath); !str.Empty())
        {
            JsonAssetReader reader(str);
            Serialization::Deserialize(typeHandler, reader, reader.ReadObject(), instance);
        }
    }
}
