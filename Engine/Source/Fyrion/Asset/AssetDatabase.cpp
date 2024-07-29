#include "AssetDatabase.hpp"

#include <memory>

#include "AssetSerialization.hpp"
#include "AssetTypes.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/IO/FileWatcher.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{
    namespace
    {
        bool RegisterEvents();

        String                         dataDirectory;
        Array<Asset*>                  assets;
        HashMap<UUID, Asset*>          assetsById;
        HashMap<String, Asset*>        assetsByPath;
        Array<Pair<TypeID, AssetIO*>>  assetIOs;
        HashMap<String, AssetIO*>      importers;
        HashMap<TypeID, Array<Asset*>> assetsByType;
        std::unique_ptr<FileWatcher>   fileWatcher;
        bool                           hotReloadEnabled = false;

        Logger& logger = Logger::GetLogger("Fyrion::AssetDatabase");
        bool    registerEvents = RegisterEvents();

        void OnTypeAddedImpl(const TypeHandler& typeHandler)
        {
            if (typeHandler.IsDerivedFrom(GetTypeID<AssetIO>()))
            {
                if (AssetIO* assetIo = typeHandler.Cast<AssetIO>(typeHandler.NewInstance()))
                {
                    assetIOs.EmplaceBack(typeHandler.GetTypeInfo().typeId, assetIo);

                    for (StringView extension : assetIo->GetImportExtensions())
                    {
                        importers.Insert(extension, assetIo);
                    }
                }
            }
        }

        bool RegisterEvents()
        {
            Event::Bind<OnTypeAdded, OnTypeAddedImpl>();
            return true;
        }
    }

    void AssetDatabaseUpdatePath(Asset* asset, const StringView& oldPath, const StringView& newPath)
    {
        if (!oldPath.Empty())
        {
            assetsByPath.Erase(oldPath);
        }
        assetsByPath.Insert(String{newPath}, asset);
        logger.Debug("asset {} registred to path {} ", asset->GetName(), newPath);
    }

    void AssetDatabaseUpdateUUID(Asset* asset, const UUID& newUUID)
    {
        if (asset->GetUUID())
        {
            assetsById.Erase(asset->GetUUID());
        }

        if (newUUID)
        {
            assetsById.Insert(newUUID, asset);
        }
    }


    Asset* AssetDatabase::FindById(const UUID& assetId)
    {
        if (!assetId) return nullptr;

        if (auto it = assetsById.Find(assetId))
        {
            return it->second;
        }

        return nullptr;
    }

    Asset* AssetDatabase::FindByPath(const StringView& path)
    {
        if (auto it = assetsByPath.Find(path))
        {
            return it->second;
        }

        return nullptr;
    }

    Span<Asset*> AssetDatabase::FindAssetsByType(TypeID typeId)
    {
        if (auto it = assetsByType.Find(typeId))
        {
            return it->second;
        }
        return {};
    }

    Asset* AssetDatabase::Create(TypeID typeId)
    {
        return Create(typeId, {});
    }

    Asset* AssetDatabase::Create(TypeID typeId, UUID uuid)
    {
        if (auto it = assetsById.Find(uuid))
        {
            return it->second;
        }

        TypeHandler* typeHandler = Registry::FindTypeById(typeId);
        FY_ASSERT(typeHandler, "type not found");

        Asset* asset = typeHandler->Cast<Asset>(typeHandler->NewInstance());
        FY_ASSERT(asset, "type cannot be casted to Asset, check if FY_BASE_TYPES(Asset) is included on the class");

        asset->uuid = uuid;
        asset->assetType = typeHandler;
        asset->loadedVersion = 0;
        asset->currentVersion = 1;
        asset->index = assets.Size();

        assets.EmplaceBack(asset);

        if (asset->uuid)
        {
            assetsById.Insert(asset->uuid, asset);
        }

        {
            auto itByType = assetsByType.Find(typeId);
            if (itByType == assetsByType.end())
            {
                itByType = assetsByType.Emplace(typeId, {}).first;
            }
            itByType->second.EmplaceBack(asset);
        }

        return asset;
    }

    Asset* AssetDatabase::CreateFromPrototype(TypeID typeId, Asset* prototype)
    {
        return CreateFromPrototype(typeId, prototype, {});
    }

    Asset* AssetDatabase::CreateFromPrototype(TypeID typeId, Asset* prototype, UUID uuid)
    {
        Asset* asset = Create(typeId, uuid);
        asset->prototype = prototype;
        return asset;
    }

    void AssetDatabase::Destroy(Asset* asset)
    {
        Asset* lastAsset = assets.Back();
        if (lastAsset != asset)
        {
            lastAsset->index = asset->index;
            assets[lastAsset->index] = lastAsset;
        }

        assets.PopBack();

        assetsById.Erase(asset->GetUUID());
        assetsByPath.Erase(asset->GetPath());

        asset->assetType->Destroy(asset);
    }

    AssetDirectory* AssetDatabase::LoadFromDirectory(const StringView& name, const StringView& directory)
    {
        if (!FileSystem::GetFileStatus(directory).exists)
        {
            return nullptr;
        }

        AssetDirectory* assetDirectory = Create<AssetDirectory>();
        assetDirectory->SetName(name);
        assetDirectory->path = String(name) + ":/";
        assetDirectory->absolutePath = directory;
        assetsByPath.Insert(assetDirectory->path, assetDirectory);

        for (const auto& entry : DirectoryEntries{directory})
        {
            LoadAssetFile(assetDirectory, entry);
        }

        assetDirectory->loadedVersion = assetDirectory->currentVersion;

        if (fileWatcher)
        {
            fileWatcher->Watch(directory);
        }

        return assetDirectory;
    }

    void AssetDatabase::LoadAssetFile(AssetDirectory* parentDirectory, const StringView& filePath)
    {
        String extension = Path::Extension(filePath);

        if (FileSystem::GetFileStatus(filePath).isDirectory)
        {
            AssetDirectory* assetDirectory = Create<AssetDirectory>();
            assetDirectory->absolutePath = filePath;
            assetDirectory->loadedVersion = assetDirectory->currentVersion;
            assetDirectory->name = Path::Name(filePath);
            assetDirectory->lastModified = FileSystem::GetFileStatus(filePath).lastModifiedTime;
            parentDirectory->AddChild(assetDirectory);

            for (const auto& entry : DirectoryEntries{filePath})
            {
                LoadAssetFile(assetDirectory, entry);
            }
        }
        else if (extension == FY_ASSET_EXTENSION)
        {
            if (Asset* asset = ReadAssetFile(filePath))
            {
                asset->name = Path::Name(filePath);
                asset->extension = Path::Extension(filePath);
                asset->absolutePath = filePath;
                asset->loadedVersion = asset->currentVersion;
                asset->lastModified = FileSystem::GetFileStatus(filePath).lastModifiedTime;
                parentDirectory->AddChild(asset);
            }
        }
        else if (auto importer = importers.Find(extension))
        {
            String infoPath = Path::Join(Path::Parent(filePath), Path::Name(filePath), FY_INFO_EXTENSION);
            if (FileSystem::GetFileStatus(infoPath).exists)
            {
                if (Asset* asset = ReadAssetFile(infoPath))
                {
                    asset->name = Path::Name(filePath);
                    asset->extension = Path::Extension(filePath);
                    asset->absolutePath = filePath;
                    asset->loadedVersion = asset->currentVersion;
                    asset->lastModified = FileSystem::GetFileStatus(filePath).lastModifiedTime;
                    parentDirectory->AddChild(asset);

                    String assetDataDir = Path::Join(dataDirectory, ToString(asset->GetUUID()));
                    if (asset->hasBlobs && !FileSystem::GetFileStatus(assetDataDir).exists)
                    {
                        importer->second->ImportAsset(filePath, asset);
                    }
                }
            }
            else if (Asset* asset = importer->second->CreateAsset())
            {
                asset->name = Path::Name(filePath);
                asset->absolutePath = filePath;
                asset->extension = Path::Extension(filePath);
                asset->lastModified = FileSystem::GetFileStatus(filePath).lastModifiedTime;

                parentDirectory->AddChild(asset);
                AssetDatabaseUpdateUUID(asset, asset->GetUUID());
                importer->second->ImportAsset(filePath, asset);
                asset->BuildPath();
            }
        }
    }

    Asset* AssetDatabase::DeserializeAsset(ArchiveReader& reader, ArchiveObject object)
    {
        if (TypeHandler* typeHandler = Registry::FindTypeByName(reader.ReadString(object, "_type")))
        {
            Asset* asset = Create(typeHandler->GetTypeInfo().typeId, UUID::FromString(reader.ReadString(object, "uuid")));
            Serialization::Deserialize(typeHandler, reader, object, asset);

            if (ArchiveObject dataObject = reader.ReadObject(object, "_data"))
            {
                asset->DeserializeData(reader, dataObject);
            }

            AssetDatabaseUpdateUUID(asset, asset->GetUUID());

            ArchiveObject arr = reader.ReadObject(object, "_assets");
            auto          size = reader.ArrSize(arr);
            ArchiveObject item{};
            for (usize i = 0; i < size; ++i)
            {
                item = reader.Next(arr, item);
                if (item)
                {
                    Asset* child = DeserializeAsset(reader, item);
                    child->SetOwner(asset);
                }
            }

            return asset;
        }

        return nullptr;
    }

    Asset* AssetDatabase::ReadAssetFile(const StringView& path)
    {
        FileHandler handler = FileSystem::OpenFile(path, AccessMode::ReadOnly);
        u64         size = FileSystem::GetFileSize(handler);
        String      buffer(size);
        FileSystem::ReadFile(handler, buffer.begin(), buffer.Size());
        FileSystem::CloseFile(handler);

        JsonAssetReader reader(buffer);
        return DeserializeAsset(reader, reader.ReadObject());
    }

    ArchiveObject AssetDatabase::SerializeAsset(ArchiveWriter& writer, Asset* asset)
    {
        ArchiveObject object = Serialization::Serialize(asset->GetAssetType(), writer, asset);
        writer.WriteString(object, "_type", asset->GetAssetType()->GetName());
        writer.WriteString(object, "uuid", ToString(asset->GetUUID()));

        if (!asset->assets.Empty())
        {
            ArchiveObject arr = writer.CreateArray();
            for (Asset* child : asset->assets)
            {
                ArchiveObject assetObj = SerializeAsset(writer, child);
                writer.WriteValue(assetObj, "_data", child->SerializeData(writer));
                writer.AddValue(arr, assetObj);
            }
            writer.WriteValue(object, "_assets", arr);
        }
        return object;
    }

    void AssetDatabase::SaveOnDirectory(AssetDirectory* directoryAsset, const StringView& directoryPath)
    {
        if (!FileSystem::GetFileStatus(directoryPath).exists)
        {
            FileSystem::CreateDirectory(directoryPath);
        }

        for (Asset* asset : directoryAsset->GetChildren())
        {
            const bool oldPathExists = FileSystem::GetFileStatus(asset->GetAbsolutePath()).exists;

            if (!asset->IsActive())
            {
                if (oldPathExists)
                {
                    String infoFile = Path::Join(Path::Parent(asset->GetAbsolutePath()), asset->GetName(), FY_INFO_EXTENSION);
                    if (FileSystem::GetFileStatus(infoFile).exists)
                    {
                        FileSystem::Remove(infoFile);
                    }

                    String assetDataDirectory = Path::Join(dataDirectory, ToString(asset->GetUUID()));
                    if (FileSystem::GetFileStatus(assetDataDirectory).exists)
                    {
                        FileSystem::Remove(assetDataDirectory);
                    }

                    if (!asset->GetDataExtesion().Empty())
                    {
                        String dataPath = Path::Join(Path::Parent(asset->GetAbsolutePath()), Path::Name(asset->GetAbsolutePath()), asset->GetDataExtesion());
                        if (FileSystem::GetFileStatus(dataPath).exists)
                        {
                            FileSystem::Remove(dataPath);
                        }
                    }

                    FileSystem::Remove(asset->GetAbsolutePath());
                    logger.Debug("Asset {} removed on {} ", asset->GetPath(), asset->GetAbsolutePath());
                    asset->absolutePath = "";
                }
            }
            else if (AssetDirectory* dir = dynamic_cast<AssetDirectory*>(asset))
            {
                String     newPath = Path::Join(directoryPath, asset->GetName());
                const bool newPathExists = FileSystem::GetFileStatus(newPath).exists;

                if (oldPathExists && !newPathExists)
                {
                    FileSystem::Rename(asset->GetAbsolutePath(), newPath);
                    logger.Debug("Directory {} moved from {} to {} ", asset->GetPath(), asset->GetAbsolutePath(), newPath);
                }
                else if (!newPathExists)
                {
                    FileSystem::CreateDirectory(newPath);
                    logger.Debug("Directory {} created on {} ", asset->GetPath(), newPath);
                }
                dir->absolutePath = newPath;
                SaveOnDirectory(dir, newPath);
            }
            else if (asset->IsModified())
            {
                String assetPath = Path::Join(directoryPath, asset->GetName(), asset->extension);

                const bool moved = assetPath != asset->GetAbsolutePath();

                if (moved && oldPathExists)
                {
                    String infoFile = Path::Join(Path::Parent(asset->GetAbsolutePath()), asset->GetName(), FY_INFO_EXTENSION);
                    if (FileSystem::GetFileStatus(infoFile).exists)
                    {
                        FileSystem::Remove(infoFile);
                    }
                    logger.Debug("Asset {} moved from {} to {} ", asset->GetPath(), asset->GetAbsolutePath(), assetPath);
                    FY_ASSERT(FileSystem::Rename(asset->GetAbsolutePath(), assetPath), "something went wrong");
                }

                if (moved && !asset->GetDataExtesion().Empty())
                {
                    String dataPath = Path::Join(Path::Parent(asset->GetAbsolutePath()), Path::Name(asset->GetAbsolutePath()), asset->GetDataExtesion());
                    if (FileSystem::GetFileStatus(dataPath).exists)
                    {
                        String newDataPath = Path::Join(Path::Parent(assetPath), Path::Name(assetPath), asset->GetDataExtesion());
                        FileSystem::Rename(dataPath, newDataPath);
                    }
                }

                String serializedPath = asset->extension == FY_ASSET_EXTENSION ? assetPath : Path::Join(directoryPath, asset->GetName(), FY_INFO_EXTENSION);

                JsonAssetWriter writer;
                String          str = JsonAssetWriter::Stringify(SerializeAsset(writer, asset));
                FileHandler     handler = FileSystem::OpenFile(serializedPath, AccessMode::WriteOnly);
                FileSystem::WriteFile(handler, str.begin(), str.Size());
                FileSystem::CloseFile(handler);

                asset->absolutePath = assetPath;
                asset->SaveData();
            }
            asset->loadedVersion = asset->currentVersion;
        }
    }

    void AssetDatabase::SetDataDirectory(const StringView& directory)
    {
        dataDirectory = directory;
        if (!FileSystem::GetFileStatus(dataDirectory).exists)
        {
            FileSystem::CreateDirectory(dataDirectory);
        }
    }

    StringView AssetDatabase::GetDataDirectory()
    {
        return dataDirectory;
    }

    void AssetDatabase::GetUpdatedAssets(AssetDirectory* directoryAsset, Array<Asset*>& updatedAssets)
    {
        if (!directoryAsset) return;

        for (Asset* asset : directoryAsset->GetChildren())
        {
            if (asset->IsModified())
            {
                updatedAssets.EmplaceBack(asset);
            }

            if (AssetDirectory* childDirectory = dynamic_cast<AssetDirectory*>(asset))
            {
                GetUpdatedAssets(childDirectory, updatedAssets);
            }
        }
    }


    AssetDirectory* AssetDatabase::LoadFromPackage(const StringView& name, const StringView& file, const StringView& binFile)
    {
        FY_ASSERT(false, "not implemented");
        return nullptr;
    }

    Asset* AssetDatabase::ImportAsset(AssetDirectory* directory, const StringView& path)
    {
        return nullptr;
    }

    bool AssetDatabase::CanReimportAsset(Asset* asset)
    {
        return importers.Find(asset->GetExtension()) != importers.end();
    }

    void AssetDatabase::ReimportAsset(Asset* asset)
    {
        if (auto importer = importers.Find(asset->GetExtension()))
        {
            importer->second->ImportAsset(asset->GetAbsolutePath(), asset);
            logger.Info("asset {} reimported", asset->GetPath());
            asset->SetModified();
        }
    }

    void AssetDatabase::DestroyAssets()
    {
        if (Engine::IsRunning())
        {
            logger.Critical("DestroyAssets() cannot be called when the engine is running");
            return;
        }

        for (Asset* asset : assets)
        {
            asset->GetAssetType()->Destroy(asset);
        }

        assets.Clear();
        assets.ShrinkToFit();

        assetsById.Clear();
        assetsByPath.Clear();
    }

    void AssetDatabase::OnUpdate(f64 deltaTime)
    {
        if (fileWatcher)
        {
            fileWatcher->CheckForUpdates([](const StringView& absolutePath, FileNotifyEvent event)
            {
                switch (event)
                {
                    case FileNotifyEvent::Added:
                    {
                        // String parent = Path::Parent(absolutePath);
                        // if (auto it = assetsByAbsolutePath.Find(parent))
                        // {
                        //     //ImportAsset()
                        //     logger.Info("file {} added on directory {} last modified time {} ", absolutePath, parent, FileSystem::GetFileStatus(absolutePath).lastModifiedTime);
                        // }
                        break;
                    }
                    case FileNotifyEvent::Removed:
                        break;
                    case FileNotifyEvent::Modified:
                        logger.Info("modified on path {} last modified time {} ", absolutePath, FileSystem::GetFileStatus(absolutePath).lastModifiedTime);
                        break;
                    case FileNotifyEvent::RenamedOld:
                        break;
                    case FileNotifyEvent::RenamedNew:
                        break;
                }
            });
        }
    }

    void AssetDatabaseInit()
    {
        if (hotReloadEnabled)
        {
            fileWatcher = std::make_unique<FileWatcher>();
        }
        Event::Bind<OnUpdate, AssetDatabase::OnUpdate>();
    }

    void AssetDatabase::EnableHotReload(bool enable)
    {
        hotReloadEnabled = enable;
    }

    void AssetDatabaseShutdown()
    {
        Event::Unbind<OnUpdate, AssetDatabase::OnUpdate>();

        hotReloadEnabled = false;
        fileWatcher = {};

        AssetDatabase::DestroyAssets();

        for (const auto& assetIo : assetIOs)
        {
            if (TypeHandler* typeHandler = Registry::FindTypeById(assetIo.first))
            {
                typeHandler->Destroy(assetIo.second);
            }
        }

        dataDirectory.Clear();

        assetIOs.Clear();
        assetIOs.ShrinkToFit();
        importers.Clear();
    }
}
