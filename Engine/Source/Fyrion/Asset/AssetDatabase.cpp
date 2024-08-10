#include "AssetDatabase.hpp"

#include <functional>
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

        String                         cacheDirectory;
        Array<Asset*>                  assets;
        HashMap<UUID, Asset*>          assetsById;
        HashMap<String, Asset*>        assetsByPath;
        Array<Pair<TypeID, AssetIO*>>  assetIOs;
        HashMap<String, AssetIO*>      importers;
        HashMap<TypeID, Array<Asset*>> assetsByType;
        FileWatcher                    fileWatcher;
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

    Asset* AssetDatabase::Create(TypeID typeId, const AssetCreation& assetCreation)
    {
        TypeHandler* typeHandler = Registry::FindTypeById(typeId);
        FY_ASSERT(typeHandler, "type not found");

        Asset* asset = typeHandler->Cast<Asset>(typeHandler->NewInstance());
        FY_ASSERT(asset, "type cannot be casted to Asset, check if FY_BASE_TYPES(Asset) is included on the class");

        if (!asset)
        {
            return nullptr;
        }

        asset->uuid = assetCreation.uuid ? assetCreation.uuid : assetCreation.generateUUID ? UUID::RandomUUID() : UUID{};
        asset->assetType = typeHandler;
        asset->loadedVersion = 0;
        asset->currentVersion = 1;
        asset->prototype = assetCreation.prototype;
        asset->name = !assetCreation.generateName ? String(assetCreation.name) : String("New").Append(" ").Append(asset->GetDisplayName());
        asset->absolutePath = assetCreation.absolutePath;
        asset->path = assetCreation.desiredPath;
        asset->parent = assetCreation.parent;

        asset->BuildPath();

        if (assetCreation.absolutePath.Empty() && asset->parent != nullptr && !asset->name.Empty())
        {
            asset->absolutePath = Path::Join(asset->parent->GetAbsolutePath(), asset->name, typeId != GetTypeID<AssetDirectory>() ? FY_ASSET_EXTENSION : "");
        }

        assets.EmplaceBack(asset);

        if (asset->parent)
        {
            asset->parent->AddChild(asset);
        }

        if (asset->uuid)
        {
            assetsById.Insert(asset->uuid, asset);
        }

        auto itByType = assetsByType.Find(typeId);
        if (itByType == assetsByType.end())
        {
            itByType = assetsByType.Emplace(typeId, {}).first;
        }

        itByType->second.EmplaceBack(asset);

        asset->OnCreated();

        if (FileSystem::GetFileStatus(asset->GetAbsolutePath()).exists)
        {
            WatchAsset(asset);
        }

        return asset;
    }

    AssetDirectory* AssetDatabase::LoadFromDirectory(const StringView& name, const StringView& directory)
    {
        if (!FileSystem::GetFileStatus(directory).exists)
        {
            return nullptr;
        }

        AssetDirectory* assetDirectory = Create<AssetDirectory>(AssetCreation{
            .name = name,
            .desiredPath = String(name) + ":/",
            .absolutePath = directory,
        });

        assetsByPath.Insert(assetDirectory->GetPath(), assetDirectory);

        for (const auto& entry : DirectoryEntries{directory})
        {
            LoadAssetFile(assetDirectory, entry);
        }

        fileWatcher.Watch(assetDirectory, directory);

        return assetDirectory;
    }

    void AssetDatabase::LoadAssetFile(AssetDirectory* parentDirectory, const StringView& filePath)
    {
        String extension = Path::Extension(filePath);
        if (FileSystem::GetFileStatus(filePath).isDirectory)
        {
            AssetDirectory* assetDirectory = Create<AssetDirectory>({
                .name = Path::Name(filePath),
                .parent = parentDirectory,
                .absolutePath = filePath,
            });

            assetDirectory->lastModifiedTime = FileSystem::GetFileStatus(filePath).lastModifiedTime;

            for (const auto& entry : DirectoryEntries{filePath})
            {
                LoadAssetFile(assetDirectory, entry);
            }

            fileWatcher.Watch(assetDirectory, filePath);
        }
        else if (extension == FY_INFO_EXTENSION)
        {
            //add it later
        }
        else if (auto importer = importers.Find(extension))
        {
            AssetIO* io = importer->second;

            Asset* asset = Create(io->GetAssetTypeID(filePath), AssetCreation{
                                      .name = Path::Name(filePath),
                                      .parent = parentDirectory,
                                      .absolutePath = filePath,
                                      .generateUUID = false,
                                  });

            String importPath = Path::Join(Path::Parent(filePath), Path::Name(filePath), FY_IMPORT_EXTENSION);
            if (!FileSystem::GetFileStatus(importPath).exists)
            {
                asset->SetUUID(UUID::RandomUUID());
                asset->lastModifiedTime = FileSystem::GetFileStatus(filePath).lastModifiedTime;
                QueueAssetImport(io, asset);
            }
            else
            {
                LoadInfoJson(importPath, asset);

                //TODO - temporary, should be lodaded under demand.
                String assetPath = Path::Join(asset->GetCacheDirectory(), asset->name, FY_ASSET_EXTENSION);
                LoadAssetJson(assetPath, asset);

                if (u64 lastModifiedTime = FileSystem::GetFileStatus(filePath).lastModifiedTime; asset->lastModifiedTime != lastModifiedTime || !FileSystem::GetFileStatus(asset->GetCacheDirectory()).exists)
                {
                    asset->lastModifiedTime = lastModifiedTime;
                    QueueAssetImport(io, asset);
                }
            }
            fileWatcher.Watch(asset, filePath);
            asset->MarkSaved();
        }

#if 0
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

                    String assetDataDir = Path::Join(cacheDirectory, ToString(asset->GetUUID()));
                    if (asset->hasBlobs && !FileSystem::GetFileStatus(assetDataDir).exists)
                    {
                        importer->second->ImportAsset(filePath, asset);
                    }
                    fileWatcher.Watch(asset, filePath);
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

                fileWatcher.Watch(asset, filePath);
            }
        }
#endif
    }


    void AssetDatabase::QueueAssetImport(AssetIO* io, Asset* asset)
    {
        //TODO enqueue to a thread pool
        io->ImportAsset(asset->absolutePath, asset);

        String assetPath = Path::Join(asset->GetCacheDirectory(), asset->name, FY_ASSET_EXTENSION);
        String infoPath = Path::Join(asset->GetParent()->GetAbsolutePath(), asset->name, FY_IMPORT_EXTENSION);
        SaveInfoJson(infoPath, asset);
        SaveAsset(assetPath, asset);
    }

    void AssetDatabase::SaveInfoJson(StringView file, Asset* asset)
    {
        JsonAssetWriter jsonAssetWriter;
        FileSystem::SaveFileAsString(file, JsonAssetWriter::Stringify(SaveInfo(jsonAssetWriter, asset)));
    }

    ArchiveObject AssetDatabase::SaveInfo(ArchiveWriter& writer, Asset* asset, bool isChild)
    {
        ArchiveObject object = writer.CreateObject();
        writer.WriteString(object, "uuid", ToString(asset->GetUUID()));
        if (asset->lastModifiedTime != 0)
        {
            writer.WriteUInt(object, "lastModifiedTime", asset->lastModifiedTime);
        }
        writer.WriteString(object, "type", asset->GetType()->GetName());
        if (isChild)
        {
            writer.WriteString(object, "name", asset->GetName());
        }
        if (ImportSettings* importSettings = asset->GetImportSettings(); importSettings != nullptr && importSettings->GetTypeHandler() != nullptr)
        {
            writer.WriteValue(object, "importSettings", Serialization::Serialize(importSettings->GetTypeHandler(), writer, importSettings));
        }
        if (!asset->GetChildren().Empty())
        {
            ArchiveObject arr = writer.CreateArray();
            for (Asset* child : asset->GetChildren())
            {
                writer.AddValue(arr, SaveInfo(writer, child, true));
            }
            writer.WriteValue(object, "children", arr);
        }
        return object;
    }

    void AssetDatabase::SaveAsset(StringView file, Asset* asset)
    {
        JsonAssetWriter writer;
        std::function<ArchiveObject(ArchiveWriter& writer, Asset* asset)> serialize;
        serialize = [&](ArchiveWriter& writer, Asset* asset)
        {
            FY_ASSERT(asset->GetUUID(), "assets without uuid cannot be serialized");

            ArchiveObject object = Serialization::Serialize(asset->GetType(), writer, asset);
            writer.WriteString(object, "_uuid", ToString(asset->GetUUID()));
            if (!asset->GetChildren().Empty())
            {
                ArchiveObject arr = writer.CreateArray();
                for (Asset* child : asset->GetChildren())
                {
                    ArchiveObject assetObj = serialize(writer, child);
                    writer.WriteValue(assetObj, "_data", child->SerializeData(writer));
                    writer.AddValue(arr, assetObj);
                }
                writer.WriteValue(object, "_children", arr);
            }
            return object;
        };
        FileSystem::SaveFileAsString(file, JsonAssetWriter::Stringify(serialize(writer, asset)));
    }

    void AssetDatabase::LoadInfoJson(StringView file, Asset* asset)
    {
        JsonAssetReader reader(FileSystem::ReadFileAsString(file));
        LoadInfo(reader, reader.ReadObject(), asset);
    }

    void AssetDatabase::LoadInfo(ArchiveReader& reader, ArchiveObject object, Asset* asset)
    {
        asset->lastModifiedTime = reader.ReadUInt(object, "lastModifiedTime");
        asset->SetUUID(UUID::FromString(reader.ReadString(object, "uuid")));

        if (ImportSettings* importSettings = asset->GetImportSettings())
        {
            if (ArchiveObject settingsObj = reader.ReadObject(object, "importSettings"))
            {
                Serialization::Deserialize(importSettings->GetTypeHandler(), reader, settingsObj, importSettings);
            }
        }

        if (ArchiveObject arr = reader.ReadObject(object, "children"))
        {
            auto size = reader.ArrSize(arr);
            ArchiveObject item{};
            for (usize i = 0; i < size; ++i)
            {
                item = reader.Next(arr, item);
                if (item)
                {
                    if (TypeHandler* typeHandler = Registry::FindTypeByName(reader.ReadString(item, "type")))
                    {
                        Asset* child = Create(typeHandler->GetTypeInfo().typeId, AssetCreation{
                            .name = reader.ReadString(item, "name"),
                            .parent = asset,
                            .generateUUID = false,
                        });
                        LoadInfo(reader, item, child);
                    }
                }
            }
        }
    }

    void AssetDatabase::LoadAssetJson(StringView file, Asset* asset)
    {
        String content = FileSystem::ReadFileAsString(file);
        if (!content.Empty())
        {
            JsonAssetReader reader(FileSystem::ReadFileAsString(file));
            LoadAsset(reader, reader.ReadObject(), asset);
        }
    }

    void AssetDatabase::LoadAsset(ArchiveReader& reader, ArchiveObject object, Asset* asset)
    {
        Serialization::Deserialize(asset->GetType(), reader, object, asset);
        ArchiveObject arr = reader.ReadObject(object, "_children");
        auto size = reader.ArrSize(arr);
        ArchiveObject item{};
        for (usize i = 0; i < size; ++i)
        {
            item = reader.Next(arr, item);
            if (item)
            {
                StringView uuidStr = reader.ReadString(item, "_uuid");
                if (Asset* childAsset = FindById(UUID::FromString(uuidStr)))
                {
                    LoadAsset(reader, item, childAsset);
                } else
                {
                    logger.Warn("child asset {} from {} not found", uuidStr, asset->GetPath());
                }
            }
        }
    }

    void AssetDatabase::SaveOnDirectory(AssetDirectory* directoryAsset, const StringView& directoryPath)
    {
        /*
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

                    String assetDataDirectory = Path::Join(cacheDirectory, ToString(asset->GetUUID()));
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

                if (dir->absolutePath != newPath)
                {
                    dir->absolutePath = newPath;
                    fileWatcher.Watch(dir, dir->absolutePath);
                }

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

                if (asset->absolutePath != assetPath)
                {
                    asset->absolutePath = assetPath;
                    fileWatcher.Watch(asset, asset->absolutePath);
                }
                asset->SaveData();
            }
            asset->loadedVersion = asset->currentVersion;
        }
        */
    }

    void AssetDatabase::SetCacheDirectory(const StringView& directory)
    {
        cacheDirectory = directory;
        if (!FileSystem::GetFileStatus(cacheDirectory).exists)
        {
            FileSystem::CreateDirectory(cacheDirectory);
        }
    }

    StringView AssetDatabase::GetCacheDirectory()
    {
        FY_ASSERT(!cacheDirectory.Empty(), "cache dir not set");
        return cacheDirectory;
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

    void AssetDatabase::ImportAsset(AssetDirectory* directory, const StringView& path)
    {
        FileSystem::CopyFile(path, Path::Join(directory->GetAbsolutePath(), Path::Name(path), Path::Extension(path)));
        fileWatcher.Check();
    }

    bool AssetDatabase::CanReimportAsset(Asset* asset)
    {
        return importers.Find(asset->GetExtension()) != importers.end();
    }

    void AssetDatabase::ReimportAsset(Asset* asset)
    {
        if (auto importer = importers.Find(asset->GetExtension()))
        {
            QueueAssetImport(importer->second, asset);
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
            asset->GetType()->Destroy(asset);
        }

        assets.Clear();
        assets.ShrinkToFit();

        assetsById.Clear();
        assetsByPath.Clear();
    }

    void AssetDatabase::OnUpdate(f64 deltaTime)
    {
        fileWatcher.CheckForUpdates([](const FileWatcherModified& modified)
        {
            switch (modified.event)
            {
                case FileNotifyEvent::Added:
                {
                    if (Asset* directory = static_cast<AssetDirectory*>(modified.userData); directory->FindChildByAbsolutePath(modified.path) == nullptr)
                    {
                        LoadAssetFile(dynamic_cast<AssetDirectory*>(directory), modified.path);
                    }
                    break;
                }
                case FileNotifyEvent::Removed:
                    static_cast<Asset*>(modified.userData)->Destroy();
                    break;
                case FileNotifyEvent::Modified:
                    ReimportAsset(static_cast<Asset*>(modified.userData));
                    break;
                case FileNotifyEvent::Renamed:
                    static_cast<Asset*>(modified.userData)->SetName(modified.name);
                    break;
            }
        });
    }

    void AssetDatabaseInit()
    {
        if (hotReloadEnabled)
        {
            fileWatcher.Start();
        }
        Event::Bind<OnUpdate, AssetDatabase::OnUpdate>();
    }

    void AssetDatabase::EnableHotReload(bool enable)
    {
        hotReloadEnabled = enable;
    }

    void AssetDatabase::WatchAsset(Asset* asset)
    {
        if (asset && hotReloadEnabled)
        {
            fileWatcher.Watch(asset, asset->GetAbsolutePath());
        }
    }

    void AssetDatabaseShutdown()
    {
        Event::Unbind<OnUpdate, AssetDatabase::OnUpdate>();

        hotReloadEnabled = false;
        fileWatcher.Stop();

        AssetDatabase::DestroyAssets();

        for (const auto& assetIo : assetIOs)
        {
            if (TypeHandler* typeHandler = Registry::FindTypeById(assetIo.first))
            {
                typeHandler->Destroy(assetIo.second);
            }
        }

        cacheDirectory.Clear();

        assetIOs.Clear();
        assetIOs.ShrinkToFit();
        importers.Clear();
    }
}
