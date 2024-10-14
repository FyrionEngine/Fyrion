
#include "JsonAssetHandler.hpp"
#include "Fyrion/Core/Repository.hpp"
#include "Fyrion/Core/StreamBuffer.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{

    namespace
    {
        bool HasBuffers(TypeHandler* typeHandler)
        {
            for (FieldHandler* fieldHandler : typeHandler->GetFields())
            {
                TypeID typeID = fieldHandler->GetFieldInfo().typeInfo.typeId;

                if (typeID == GetTypeID<StreamBuffer>())
                {
                    return true;
                }

                if (TypeHandler* fieldTypeHandler = Registry::FindTypeById(typeID))
                {
                    if (HasBuffers(fieldTypeHandler))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        void FlushBuffers(OutputFileStream& stream, TypeHandler* typeHandler, VoidPtr instance)
        {
            for (FieldHandler* fieldHandler : typeHandler->GetFields())
            {
                VoidPtr fieldPointer = fieldHandler->GetFieldPointer(instance);
                TypeID typeID = fieldHandler->GetFieldInfo().typeInfo.typeId;

                if (typeID == GetTypeID<StreamBuffer>())
                {
                    StreamBuffer& buffer = *static_cast<StreamBuffer*>(fieldPointer);
                    auto data = buffer.GetInMemoryData();
                    buffer.offset = stream.Write(data.begin(), data.Size());
                    buffer.size = data.Size();
                    buffer.inMemoryData.Clear();
                    buffer.bufferFile = stream.GetFile();
                }

                if (TypeHandler* fieldTypeHandler = Registry::FindTypeById(typeID))
                {
                    FlushBuffers(stream, fieldTypeHandler, fieldPointer);
                }
            }
        }

        void RegisterBuffers(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance)
        {
            String bufferPath = Path::Join(assetFile->absolutePath, ".buffer");

            for (FieldHandler* fieldHandler : typeHandler->GetFields())
            {
                VoidPtr fieldPointer = fieldHandler->GetFieldPointer(instance);
                TypeID typeID = fieldHandler->GetFieldInfo().typeInfo.typeId;

                if (typeID == GetTypeID<StreamBuffer>())
                {
                    StreamBuffer& buffer = *static_cast<StreamBuffer*>(fieldPointer);
                    buffer.bufferFile = bufferPath;
                }

                if (TypeHandler* fieldTypeHandler = Registry::FindTypeById(typeID))
                {
                    RegisterBuffers(assetFile, fieldTypeHandler, fieldPointer);
                }
            }
        }
    }


    void JsonAssetHandler::Save(StringView newPath, AssetFile* assetFile)
    {
        VoidPtr      asset = Repository::Load(assetFile->uuid);
        TypeHandler* typeHandler = Repository::GetTypeHandler(assetFile->uuid);

        if (HasBuffers(typeHandler))
        {
            String bufferPath = Path::Join(newPath, ".buffer");
            OutputFileStream stream;
            stream.Open(bufferPath);
            FlushBuffers(stream, typeHandler, asset);
            stream.Close();
        }

        JsonAssetWriter writer;
        if (ArchiveObject assetArchive = Serialization::Serialize(typeHandler, writer, asset))
        {
            FileSystem::SaveFileAsString(newPath, JsonAssetWriter::Stringify(assetArchive));
        }
    }

    void JsonAssetHandler::Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance)
    {
        JsonAssetReader reader(FileSystem::ReadFileAsString(assetFile->absolutePath));
        Serialization::Deserialize(typeHandler, reader, reader.ReadObject(), instance);

        RegisterBuffers(assetFile, typeHandler, instance);
    }
}
