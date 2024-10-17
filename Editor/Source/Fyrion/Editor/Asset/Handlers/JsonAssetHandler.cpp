#include "JsonAssetHandler.hpp"

#include "Fyrion/Core/Serialization.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/IO/FileSystem.hpp"

namespace Fyrion
{
    void JsonAssetHandler::Save(StringView newPath, AssetFile* assetFile)
    {
        JsonArchiveWriter writer;
        ArchiveValue      assetArchive = Serialization::Serialize(GetAssetTypeID(), writer, Assets::Load(assetFile->uuid));
        FileSystem::SaveFileAsString(newPath, JsonArchiveWriter::Stringify(assetArchive));
    }

    void JsonAssetHandler::Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance)
    {
        if (String str = FileSystem::ReadFileAsString(assetFile->absolutePath); !str.Empty())
        {
            JsonArchiveReader reader(str);
            Serialization::Deserialize(typeHandler, reader, reader.GetRootObject(), instance);
        }
    }
}
