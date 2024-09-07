#include "AssetHandler.hpp"

#include "Asset.hpp"
#include "AssetTypes.hpp"
#include "AssetSerialization.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void AssetManagerAddHandler(AssetHandler* assetHandler);
    void AssetManagerUpdateUUID(AssetHandler* assetHandler, const UUID& newUUID);
    void AssetManagerUpdatePath(AssetHandler* assetHandler, const StringView& oldPath, const StringView& newPath);
    void AssetManagerUpdateType(AssetHandler* assetHandler, TypeHandler* typeHandler);
    void AssetManagerCleanRefs(AssetHandler* assetHandler);

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::AssetHandler");
    }

    Asset* AssetHandler::GetInstance() const
    {
        return instance;
    }

    UUID AssetHandler::GetUUID() const
    {
        return uuid;
    }

    void AssetHandler::SetUUID(UUID uuid)
    {
        FY_ASSERT(!this->uuid, "UUID cannot be changed");
        FY_ASSERT(uuid, "UUID cannot be zero");
        if (!this->uuid)
        {
            AssetManagerUpdateUUID(this, uuid);
            this->uuid = uuid;
        }
    }

    TypeHandler* AssetHandler::GetType() const
    {
        return type;
    }

    void AssetHandler::SetType(TypeHandler* typeHandler)
    {
        type = typeHandler;
        AssetManagerUpdateType(this, typeHandler);
    }

    StringView AssetHandler::GetName() const
    {
        return name;
    }

    void AssetHandler::SetName(StringView desiredNewName)
    {
        if (name != desiredNewName)
        {
            String newName = ValidateName(parent, this, desiredNewName);
            AssetMoved(newName, parent);
            logger.Debug("asset {} renamed to {} ", this->name, newName);
            name = newName;
            UpdatePath();
        }
    }

    void AssetHandler::SetParent(AssetHandler* parent)
    {
        if (this->parent != nullptr)
        {
            if (this->parent != parent)
            {
                this->parent->RemoveChild(this);
                String newName = ValidateName(parent, this, name);
                AssetMoved(newName, parent);
                name = newName;
                UpdatePath();
                parent->AddChild(this);
            }
            else
            {
                String newName = ValidateName(parent, this, name);
                AssetMoved(newName, parent);
                name = newName;
                UpdatePath();
            }
        }

        this->parent = parent;

        for (AssetHandler* child : children)
        {
            //same parent, but force to update the children to the new path.
            child->SetParent(this);
        }
    }

    StringView AssetHandler::GetPath() const
    {
        return relativePath;
    }

    StringView AssetHandler::GetDataPath()
    {
        return {};
    }

    StringView AssetHandler::GetExtension() const
    {
        return Path::Extension(GetAbsolutePath());
    }

    AssetHandler* AssetHandler::GetParent() const
    {
        return parent;
    }

    Span<AssetHandler*> AssetHandler::GetChildren() const
    {
        return children;
    }

    Asset* AssetHandler::LoadInstance()
    {
        return nullptr;
    }

    void AssetHandler::UnloadInstance()
    {
        if (instance && type)
        {
            type->Destroy(instance);
            instance = nullptr;
        }
    }

    AssetBufferManager* AssetHandler::GetBufferManager()
    {
        return nullptr;
    }

    StringView AssetHandler::GetDisplayName(TypeHandler* type)
    {
        if (type)
        {
            if (const AssetMeta* meta = type->GetAttribute<AssetMeta>(); meta != nullptr && !meta->displayName.Empty())
            {
                return meta->displayName;
            }
            return type->GetSimpleName();
        }
        return "Asset";
    }

    String AssetHandler::ValidateName(AssetHandler* parent, AssetHandler* asset, StringView newName)
    {
        u32    count{};
        String finalName = newName;
        bool   nameFound;
        do
        {
            nameFound = true;
            for (AssetHandler* child : parent->GetChildren())
            {
                if (child == asset) continue;

                if (finalName == child->name)
                {
                    finalName = newName;
                    finalName += " (";
                    finalName.Append(++count);
                    finalName += ")";
                    nameFound = false;
                    break;
                }
            }
        }
        while (!nameFound);

        return finalName;
    }

    StringView AssetHandler::GetDisplayName() const
    {
        return GetDisplayName(type);
    }

    void AssetHandler::UpdatePath()
    {
        if (parent != nullptr && !name.Empty())
        {
            name = ValidateName(parent, this, name);

            String newPath = String().Append(parent->GetPath()).Append("/").Append(name).Append(GetExtension());
            if (relativePath != newPath)
            {
                AssetManagerUpdatePath(this, relativePath, newPath);
            }
            relativePath = newPath;
        }
        else if (!relativePath.Empty())
        {
            AssetManagerUpdatePath(this, relativePath, relativePath);
        }
    }

    ArchiveObject AssetHandler::Serialize(ArchiveWriter& writer) const
    {
        ArchiveObject object = writer.CreateObject();
        writer.WriteString(object, "uuid", ToString(GetUUID()));
        writer.WriteString(object, "type", GetType()->GetName());

        if (!children.Empty())
        {
            ArchiveObject arr = writer.CreateArray();
            for (AssetHandler* child : children)
            {
                writer.AddValue(arr, child->Serialize(writer));
            }
            writer.WriteValue(object, "children", arr);
        }
        return object;
    }

    void AssetHandler::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        SetType(Registry::FindTypeByName(reader.ReadString(object, "type")));
        SetUUID(UUID::FromString(reader.ReadString(object, "uuid")));

        if (ArchiveObject arr = reader.ReadObject(object, "children"))
        {
            auto size = reader.ArrSize(arr);

            ArchiveObject item{};
            for (usize i = 0; i < size; ++i)
            {
                item = reader.Next(arr, item);
                if (item)
                {
                    AssetHandler* child = this->CreateChild(reader.ReadString(item, "name"));
                    child->Deserialize(reader, item);
                }
            }
        }
    }

    bool AssetHandler::IsModified()
    {
        return false;
    }

    void AssetHandler::SetModified()
    {
        //empty
    }

    void AssetHandler::AddRelatedFile(StringView fileAbsolutePath)
    {
        //empty
    }

    Span<String> AssetHandler::GetRelatedFiles()
    {
        return {};
    }

    void AssetHandler::RemoveChild(AssetHandler* child)
    {
        if (AssetHandler** it = FindFirst(children.begin(), children.end(), child))
        {
            children.Erase(it);
        }
    }

    void AssetHandler::RemoveFromParent()
    {
        if (parent != nullptr)
        {
            parent->RemoveChild(this);
        }
        parent = nullptr;
    }

    bool AssetHandler::IsChildOf(AssetHandler* parent) const
    {
        if (parent == this) return true;

        if (this->parent != nullptr)
        {
            if (this->parent == parent)
            {
                return true;
            }

            return this->parent->IsChildOf(parent);
        }
        return false;
    }

    void AssetHandler::AddChild(AssetHandler* child)
    {
        child->RemoveFromParent();
        child->parent = this;
        child->UpdatePath();
        children.EmplaceBack(child);
    }

    AssetHandler* AssetHandler::FindChildByAbsolutePath(StringView absolutePath) const
    {
        for (AssetHandler* child : children)
        {
            if (child->GetAbsolutePath() == absolutePath)
            {
                return child;
            }
        }
        return nullptr;
    }

    StringView DirectoryAssetHandler::GetAbsolutePath() const
    {
        return absolutePath;
    }

    void DirectoryAssetHandler::Save()
    {
        if (!FileSystem::GetFileStatus(absolutePath).exists)
        {
            FileSystem::CreateDirectory(absolutePath);
        }
    }

    void DirectoryAssetHandler::Delete()
    {
        if (!active) return;
        active = false;

        FileSystem::Remove(absolutePath);

        if (parent)
        {
            parent->RemoveChild(this);
        }

        AssetManagerCleanRefs(this);
    }

    Asset* DirectoryAssetHandler::LoadInstance()
    {
        return nullptr;
    }

    void DirectoryAssetHandler::AssetMoved(StringView newName, AssetHandler* newParent)
    {
        String newAbsolutePath = Path::Join(newParent->GetAbsolutePath(), newName);
        FileSystem::Rename(absolutePath, newAbsolutePath);
        absolutePath = newAbsolutePath;
        logger.Debug("directory {} moved to {} ", name, newName);

        AssetManager::WatchAsset(this);
    }

    AssetHandler* DirectoryAssetHandler::CreateChild(StringView name)
    {
        return Create(name, Path::Join(absolutePath, name), this);
    }

    DirectoryAssetHandler* DirectoryAssetHandler::Create(const StringView& name, const StringView& absolutePath, DirectoryAssetHandler* parent)
    {
        DirectoryAssetHandler* handler = MemoryGlobals::GetDefaultAllocator().Alloc<DirectoryAssetHandler>();
        handler->name = name;
        handler->relativePath = String(name) + ":/",
            handler->absolutePath = absolutePath;
        handler->parent = parent;

        if (parent)
        {
            parent->children.EmplaceBack(handler);
        }

        handler->UpdatePath();

        AssetManagerAddHandler(handler);

        return handler;
    }

    StringView ChildAssetHandler::GetAbsolutePath() const
    {
        return {};
    }

    void ChildAssetHandler::Save()
    {
        if (!dataPath.Empty())
        {
            String assetPath = Path::Join(dataPath, name, FY_ASSET_EXTENSION);
            logger.Debug("saving child asset on {} ", assetPath);
            JsonAssetWriter writer;
            FileSystem::SaveFileAsString(assetPath, JsonAssetWriter::Stringify(LoadInstance()->Serialize(writer)));
        }

        for (AssetHandler* handler : GetChildren())
        {
            handler->Save();
        }
    }

    void ChildAssetHandler::Delete()
    {
        if (instance)
        {
            instance->OnDestroyed();
            //TODO - dependencies are not tracked, so destroying any instance could cause an error.
            //AssetManager::UnloadAsset(this);
        }

        FileSystem::Remove(dataPath);
        logger.Debug("{} deleted", dataPath);

        AssetManagerCleanRefs(this);
    }

    AssetHandler* ChildAssetHandler::CreateChild(StringView name)
    {
        return nullptr;
    }

    void ChildAssetHandler::UpdatePath()
    {
        //ChildAssetHandler should not have paths.
    }

    StringView ChildAssetHandler::GetDataPath()
    {
        if (dataPath.Empty())
        {
            dataPath = Path::Join(parent->GetDataPath(), ToString(GetUUID()));
        }
        return dataPath;
    }

    Asset* ChildAssetHandler::LoadInstance()
    {
        if (!instance && GetType() != nullptr)
        {
            instance = GetType()->Cast<Asset>(GetType()->NewInstance());
            instance->SetHandler(this);

            String assetPath = Path::Join(GetDataPath(), name, FY_ASSET_EXTENSION);
            if (FileSystem::GetFileStatus(assetPath).exists)
            {
                if (const String str = FileSystem::ReadFileAsString(assetPath); !str.Empty())
                {
                    JsonAssetReader reader(str);
                    instance->Deserialize(reader, reader.ReadObject());
                }
            }
        }
        return instance;
    }

    ArchiveObject ChildAssetHandler::Serialize(ArchiveWriter& writer) const
    {
        ArchiveObject object = AssetHandler::Serialize(writer);
        writer.WriteString(object, "name", name);
        return object;
    }

    void ChildAssetHandler::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        if (StringView name = reader.ReadString(object, "name"); !name.Empty())
        {
            SetName(name);
        }
        AssetHandler::Deserialize(reader, object);
    }

    AssetBufferManager* ChildAssetHandler::GetBufferManager()
    {
        return &bufferManager;
    }

    ChildAssetHandler* ChildAssetHandler::Create(StringView name, AssetHandler* parent)
    {
        ChildAssetHandler* handler = MemoryGlobals::GetDefaultAllocator().Alloc<ChildAssetHandler>();
        handler->name = name;
        parent->AddChild(handler);
        AssetManagerAddHandler(handler);
        return handler;
    }

    void FileAssetBufferManager::SaveBuffer(AssetBuffer& buffer, ConstPtr data, usize dataSize)
    {
        if (!buffer)
        {
            buffer.id = Random::Xorshift64star();
        }

        StringView dataPath = assetHandler->GetDataPath();
        if (!dataPath.Empty())
        {
            if (!FileSystem::GetFileStatus(dataPath).exists)
            {
                FileSystem::CreateDirectory(dataPath);
            }
            String      bufferPath = Path::Join(dataPath, buffer.ToString());
            FileHandler file = FileSystem::OpenFile(bufferPath, AccessMode::WriteOnly);
            FileSystem::WriteFile(file, data, dataSize);
            FileSystem::CloseFile(file);
        }
    }

    Array<u8> FileAssetBufferManager::LoadBuffer(const AssetBuffer& buffer) const
    {
        String dataDir = assetHandler->GetDataPath();
        if (!dataDir.Empty())
        {
            String      bufferPath = Path::Join(dataDir, buffer.ToString());
            FileHandler file = FileSystem::OpenFile(bufferPath, AccessMode::ReadOnly);
            Array<u8>   data(FileSystem::GetFileSize(file));
            FileSystem::ReadFile(file, data.Data(), data.Size());
            FileSystem::CloseFile(file);
            return data;
        }
        return {};
    }

    bool FileAssetBufferManager::HasBuffer(AssetBuffer& buffer) const
    {
        String dataDir = assetHandler->GetDataPath();
        if (!dataDir.Empty())
        {
            String bufferPath = Path::Join(dataDir, buffer.ToString());
            return FileSystem::GetFileStatus(bufferPath).exists;
        }
        return false;
    }

    void JsonAssetHandler::AssetMoved(StringView newName, AssetHandler* newParent)
    {
        String newAssetPath = Path::Join(newParent->GetAbsolutePath(), newName, FY_ASSET_EXTENSION);
        String newInfoPath = Path::Join(newParent->GetAbsolutePath(), newName, FY_INFO_EXTENSION);
        String newDataPath = Path::Join(newParent->GetAbsolutePath(), newName, FY_DATA_EXTENSION);

        FileSystem::Rename(assetPath, newAssetPath);
        FileSystem::Rename(infoPath, newInfoPath);
        FileSystem::Rename(dataPath, newDataPath);

        assetPath = newAssetPath;
        infoPath = newInfoPath;
        dataPath = newDataPath;
    }

    StringView JsonAssetHandler::GetAbsolutePath() const
    {
        return assetPath;
    }

    bool JsonAssetHandler::IsModified()
    {
        return currentVersion != persistedVersion;
    }

    void JsonAssetHandler::SetModified()
    {
        currentVersion += 1;
    }

    void JsonAssetHandler::Save()
    {
        logger.Debug("saving info on {} ", infoPath);
        logger.Debug("saving asset on {} ", assetPath);

        JsonAssetWriter writer;
        FileSystem::SaveFileAsString(infoPath, JsonAssetWriter::Stringify(Serialize(writer)));
        FileSystem::SaveFileAsString(assetPath, JsonAssetWriter::Stringify(LoadInstance()->Serialize(writer)));

        for (AssetHandler* handler : GetChildren())
        {
            handler->Save();
        }
        persistedVersion = currentVersion;
    }

    void JsonAssetHandler::Delete()
    {
        if (!active) return;
        active = false;

        if (instance)
        {
            instance->OnDestroyed();
            //TODO - dependencies are not tracked, so destroying any instance could cause an error.
            //AssetManager::UnloadAsset(this);
        }

        FileSystem::Remove(infoPath);
        FileSystem::Remove(assetPath);

        if (parent)
        {
            parent->RemoveChild(this);
        }

        AssetManagerCleanRefs(this);
    }

    Asset* JsonAssetHandler::LoadInstance()
    {
        if (instance == nullptr && GetType() != nullptr)
        {
            instance = GetType()->Cast<Asset>(GetType()->NewInstance());
            instance->SetHandler(this);

            if (FileSystem::GetFileStatus(assetPath).exists)
            {
                if (const String str = FileSystem::ReadFileAsString(assetPath); !str.Empty())
                {
                    JsonAssetReader reader(str);
                    instance->Deserialize(reader, reader.ReadObject());
                }
            }
        }
        return instance;
    }

    StringView JsonAssetHandler::GetDataPath()
    {
        return dataPath;
    }

    AssetHandler* JsonAssetHandler::CreateChild(StringView name)
    {
        FY_ASSERT(false, "not supported yet");
        return nullptr;
    }

    AssetBufferManager* JsonAssetHandler::GetBufferManager()
    {
        return &bufferManager;
    }

    JsonAssetHandler* JsonAssetHandler::Create(StringView name, DirectoryAssetHandler* directory)
    {
        FY_ASSERT(directory, "assets must have directories");
        if (!directory) return nullptr;

        JsonAssetHandler* handler = MemoryGlobals::GetDefaultAllocator().Alloc<JsonAssetHandler>();
        handler->parent = directory;
        handler->currentVersion = 1;
        handler->name = name;
        handler->infoPath = Path::Join(directory->GetAbsolutePath(), handler->name, FY_INFO_EXTENSION);
        handler->assetPath = Path::Join(directory->GetAbsolutePath(), handler->name, FY_ASSET_EXTENSION);
        handler->dataPath = Path::Join(directory->GetAbsolutePath(), handler->name, FY_DATA_EXTENSION);

        directory->AddChild(handler);

        if (FileSystem::GetFileStatus(handler->infoPath).exists)
        {
            if (const String str = FileSystem::ReadFileAsString(handler->infoPath); !str.Empty())
            {
                JsonAssetReader reader(str);
                handler->Deserialize(reader, reader.ReadObject());
            }
            handler->persistedVersion = handler->currentVersion;
        }

        handler->UpdatePath();
        AssetManagerAddHandler(handler);
        return handler;
    }

    StringView ImportedAssetHandler::GetAbsolutePath() const
    {
        return importedFilePath;
    }

    void ImportedAssetHandler::AddRelatedFile(StringView fileAbsolutePath)
    {
        relatedFiles.EmplaceBack(fileAbsolutePath);
    }

    Span<String> ImportedAssetHandler::GetRelatedFiles()
    {
        return relatedFiles;
    }

    void ImportedAssetHandler::Save()
    {
        logger.Debug("saving imported info on {} ", infoPath);
        logger.Debug("saving imported asset on {} ", assetPath);

        JsonAssetWriter writer;
        FileSystem::SaveFileAsString(infoPath, JsonAssetWriter::Stringify(Serialize(writer)));
        FileSystem::SaveFileAsString(assetPath, JsonAssetWriter::Stringify(LoadInstance()->Serialize(writer)));

        for (AssetHandler* handler : GetChildren())
        {
            handler->Save();
        }
    }

    void ImportedAssetHandler::Delete()
    {
        if (!active) return;
        active = false;

        Array<AssetHandler*> childrenCopy = children;
        for (AssetHandler* child : childrenCopy)
        {
            child->Delete();
        }

        if (instance)
        {
            instance->OnDestroyed();
            //TODO - dependencies are not tracked, so destroying any instance could cause an error.
            //AssetManager::UnloadAsset(this);
        }

        FileSystem::Remove(infoPath);
        FileSystem::Remove(dataPath);
        FileSystem::Remove(assetPath);
        FileSystem::Remove(importedFilePath);

        logger.Debug("{} deleted", importedFilePath);
        logger.Debug("{} deleted", infoPath);
        logger.Debug("{} deleted", dataPath);
        logger.Debug("{} deleted", assetPath);

        for (const String& relatedFile : relatedFiles)
        {
            FileSystem::Remove(relatedFile);
            logger.Debug("{} deleted", relatedFile);
        }

        if (parent)
        {
            parent->RemoveChild(this);
        }

        AssetManagerCleanRefs(this);
    }

    Asset* ImportedAssetHandler::LoadInstance()
    {
        if (instance == nullptr && GetType() != nullptr)
        {
            instance = GetType()->Cast<Asset>(GetType()->NewInstance());
            instance->SetHandler(this);

            if (pendingImport)
            {
                u64 lastModifiedTime = FileSystem::GetFileStatus(importedFilePath).lastModifiedTime;
                this->lastModifiedTime = lastModifiedTime;
                AssetManager::QueueAssetImport(io, this);
                pendingImport = false;
            }

            if (FileSystem::GetFileStatus(assetPath).exists)
            {
                if (const String str = FileSystem::ReadFileAsString(assetPath); !str.Empty())
                {
                    JsonAssetReader reader(str);
                    instance->Deserialize(reader, reader.ReadObject());
                }
            }
        }
        return instance;
    }

    StringView ImportedAssetHandler::GetDataPath()
    {
        return dataPath;
    }

    ArchiveObject ImportedAssetHandler::Serialize(ArchiveWriter& writer) const
    {
        ArchiveObject object = AssetHandler::Serialize(writer);

        if (lastModifiedTime != 0)
        {
            writer.WriteUInt(object, "lastModifiedTime", lastModifiedTime);
        }

        if (!relatedFiles.Empty())
        {
            ArchiveObject arr = writer.CreateArray();
            for (const String& file : relatedFiles)
            {
                writer.AddString(arr, file);
            }
            writer.WriteValue(object, "relatedFiles", arr);
        }

        return object;
    }

    void ImportedAssetHandler::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        AssetHandler::Deserialize(reader, object);
        lastModifiedTime = reader.ReadUInt(object, "lastModifiedTime");

        if (ArchiveObject arr = reader.ReadObject(object, "relatedFiles"))
        {
            auto size = reader.ArrSize(arr);

            ArchiveObject item{};
            for (usize i = 0; i < size; ++i)
            {
                item = reader.Next(arr, item);
                if (item)
                {
                    relatedFiles.EmplaceBack(reader.GetString(item));
                }
            }
        }
    }

    AssetBufferManager* ImportedAssetHandler::GetBufferManager()
    {
        return &bufferManager;
    }

    void ImportedAssetHandler::AssetMoved(StringView newName, AssetHandler* newParent)
    {
        String newInfoPath = Path::Join(newParent->GetAbsolutePath(), newName, FY_IMPORT_EXTENSION);
        String newImportedFilePath = Path::Join(newParent->GetAbsolutePath(), newName, Path::Extension(importedFilePath));
        String newAssetPath = Path::Join(dataPath, newName, FY_ASSET_EXTENSION);

        FileSystem::Rename(infoPath, newInfoPath);
        FileSystem::Rename(assetPath, newAssetPath);
        FileSystem::Rename(importedFilePath, newImportedFilePath);

        if (io->renameAsset)
        {
            io->renameAsset(this, newName, newParent);
        }

        assetPath = newAssetPath;
        infoPath = newInfoPath;
        importedFilePath = newImportedFilePath;

        AssetManager::WatchAsset(this);
    }

    ImportedAssetHandler* ImportedAssetHandler::Create(AssetIO* io, StringView importedFilePath, DirectoryAssetHandler* directory)
    {
        ImportedAssetHandler* handler = MemoryGlobals::GetDefaultAllocator().Alloc<ImportedAssetHandler>();
        handler->SetType(Registry::FindTypeById(io->getAssetTypeId(importedFilePath)));
        handler->io = io;
        handler->name = Path::Name(importedFilePath);
        handler->importedFilePath = importedFilePath;
        directory->AddChild(handler);
        handler->infoPath = Path::Join(Path::Parent(importedFilePath), Path::Name(importedFilePath), FY_IMPORT_EXTENSION);

        bool infoLoaded = false;

        if (FileSystem::GetFileStatus(handler->infoPath).exists)
        {
            if (const String str = FileSystem::ReadFileAsString(handler->infoPath); !str.Empty())
            {
                JsonAssetReader reader(str);
                handler->Deserialize(reader, reader.ReadObject());
                infoLoaded = true;
            }
        }

        if (!handler->GetUUID())
        {
            handler->SetUUID(UUID::RandomUUID());
        }

        handler->dataPath = Path::Join(AssetManager::GetDataDirectory(), ToString(handler->GetUUID()));
        handler->assetPath = Path::Join(handler->dataPath, handler->name, FY_ASSET_EXTENSION);

        u64 lastModifiedTime = FileSystem::GetFileStatus(importedFilePath).lastModifiedTime;

        if (!infoLoaded || handler->lastModifiedTime != lastModifiedTime || !FileSystem::GetFileStatus(handler->assetPath).exists)
        {
            handler->pendingImport = true;
        }

        handler->UpdatePath();
        AssetManagerAddHandler(handler);

        return handler;
    }

    AssetHandler* ImportedAssetHandler::CreateChild(StringView name)
    {
        ChildAssetHandler* handler = ChildAssetHandler::Create(name, this);
        return handler;
    }
}
