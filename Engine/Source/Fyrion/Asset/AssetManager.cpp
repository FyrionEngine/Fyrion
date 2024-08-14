#include "AssetManager.hpp"

#include <functional>
#include <memory>

#include "AssetSerialization.hpp"
#include "AssetTypes.hpp"
#include "Fyrion/Engine.hpp"
#include "../IO/FileWatcher.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{
    namespace
    {
        bool RegisterEvents();

        String                             cacheDirectory;
        Array<AssetInfo*>                  assets;
        HashMap<UUID, AssetInfo*>          assetsById;
        HashMap<String, AssetInfo*>        assetsByPath;
        Array<Pair<TypeID, AssetIO*>>      assetIOs;
        HashMap<String, AssetIO*>          importers;
        HashMap<TypeID, Array<AssetInfo*>> assetsByType;
        FileWatcher                        fileWatcher;
        bool                               hotReloadEnabled = false;

        Logger& logger = Logger::GetLogger("Fyrion::AssetManager", LogLevel::Debug);

        bool  registerEvents = RegisterEvents();

        void OnTypeAddedImpl(const TypeHandler& typeHandler)
        {
            if (typeHandler.IsDerivedFrom(GetTypeID<AssetIO>()))
            {
                if (AssetIO* assetIo = typeHandler.Cast<AssetIO>(typeHandler.NewInstance()))
                {
                    assetIOs.EmplaceBack(typeHandler.GetTypeInfo().typeId, assetIo);

                    if (assetIo->getImportExtensions != nullptr)
                    {
                        for (StringView extension : assetIo->getImportExtensions())
                        {
                            importers.Insert(extension, assetIo);
                        }
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

    void AssetDatabaseCleanRefs(AssetInfo* assetInfo)
    {
        assetsById.Erase(assetInfo->GetUUID());
        assetsByPath.Erase(assetInfo->GetPath());

        if (auto it = assetsByType.Find(assetInfo->GetType()->GetTypeInfo().typeId))
        {
            if (const auto itArr = FindFirst(it->second.begin(), it->second.end(), assetInfo))
            {
                it->second.Erase(itArr);
            }
        }
    }

    void AssetDatabaseUpdatePath(AssetInfo* assetInfo, const StringView& oldPath, const StringView& newPath)
    {
        if (!oldPath.Empty())
        {
            assetsByPath.Erase(oldPath);
        }
        assetsByPath.Insert(String{newPath}, assetInfo);
        logger.Debug("asset {} registred to path {} ", assetInfo->GetName(), newPath);
    }

    void AssetDatabaseUpdateUUID(AssetInfo* assetInfo, const UUID& newUUID)
    {
        if (assetInfo->GetUUID())
        {
            assetsById.Erase(assetInfo->GetUUID());
        }

        if (newUUID)
        {
            assetsById.Insert(newUUID, assetInfo);
        }
    }


    Asset* AssetManager::LoadById(const UUID& assetId)
    {
        if (auto it = assetsById.Find(assetId))
        {
            return LoadAsset(it->second);
        }
        return nullptr;
    }

    Asset* AssetManager::LoadByPath(const StringView& path)
    {
        if (auto it = assetsByPath.Find(path))
        {
            return LoadAsset(it->second);
        }
        return nullptr;
    }

    Span<Asset*> AssetManager::FindAssetsByType(TypeID typeId)
    {
        // if (auto it = assetsByType.Find(typeId))
        // {
        //     return it->second;
        // }

        return {};
    }

    Asset* AssetManager::Create(TypeHandler* typeHandler, const AssetCreation& assetCreation)
    {
        TypeID typeId = typeHandler->GetTypeInfo().typeId;

        AssetInfo* assetInfo = CreateAssetInfo();
        assetInfo->name = !assetCreation.name.Empty() ? String(assetCreation.name) : String("New").Append(" ").Append(assetInfo->GetDisplayName());
        assetInfo->absolutePath = assetCreation.absolutePath;
        assetInfo->type = typeHandler;
        assetInfo->persistedVersion = 0;
        assetInfo->currentVersion = 1;
        assetInfo->relativePath = assetCreation.desiredPath;

        if (UUID newUUID = assetCreation.uuid ? assetCreation.uuid : assetCreation.generateUUID ? UUID::RandomUUID() : UUID{})
        {
            assetInfo->SetUUID(newUUID);
        }

        if (assetCreation.parent && assetCreation.parent->info)
        {
            assetCreation.parent->info->AddChild(assetInfo);
        }

        if (assetCreation.absolutePath.Empty() && assetInfo->parent != nullptr && !assetInfo->name.Empty())
        {
            assetInfo->absolutePath = Path::Join(assetInfo->parent->GetAbsolutePath(), assetInfo->name, typeId != GetTypeID<DirectoryAsset>() ? FY_ASSET_EXTENSION : "");
        }

        if (typeId != GetTypeID<DirectoryAsset>())
        {
            assetInfo->infoPath = Path::Join(assetInfo->parent->GetAbsolutePath(), assetInfo->name, FY_INFO_EXTENSION);
            assetInfo->payloadPath = assetInfo->absolutePath; //not sure?
        }

        FileStatus status = FileSystem::GetFileStatus(assetCreation.absolutePath);
        if (status.exists)
        {
            assetInfo->lastModifiedTime = status.lastModifiedTime;
        }

        auto itByType = assetsByType.Find(typeId);
        if (itByType == assetsByType.end())
        {
            itByType = assetsByType.Emplace(typeId, {}).first;
        }
        itByType->second.EmplaceBack(assetInfo);

        if (assetInfo->uuid)
        {
            assetsById.Insert(assetInfo->uuid, assetInfo);
        }

        Asset* asset = LoadAsset(assetInfo);
        assetInfo->UpdatePath();

        if (!status.exists)
        {
            asset->OnCreated();
        }

        return asset;
    }

    DirectoryAsset* AssetManager::LoadFromDirectory(const StringView& name, const StringView& directory)
    {
        if (!FileSystem::GetFileStatus(directory).exists)
        {
            return nullptr;
        }

        DirectoryAsset* directoryAsset = Create<DirectoryAsset>({
            .name = name,
            .desiredPath = String(name) + ":/",
            .absolutePath = directory
        });

        directoryAsset->info->persistedVersion = directoryAsset->info->currentVersion;

        for (const auto& entry : DirectoryEntries{directory})
        {
            LoadAssetFile(directoryAsset, entry);
        }

        fileWatcher.Watch(directoryAsset->info, directory);

        return directoryAsset;
    }

    void AssetManager::LoadAssetFile(DirectoryAsset* parentDirectory, const StringView& filePath)
    {
        String extension = Path::Extension(filePath);
        if (extension == FY_DATA_EXTENSION) return;

        if (FileSystem::GetFileStatus(filePath).isDirectory)
        {
            DirectoryAsset* directoryAsset = Create<DirectoryAsset>({
                .name = Path::Name(filePath),
                .parent = parentDirectory,
                .absolutePath = filePath,
            });

            directoryAsset->info->persistedVersion = directoryAsset->info->currentVersion;

            for (const auto& entry : DirectoryEntries{filePath})
            {
                LoadAssetFile(directoryAsset, entry);
            }

            fileWatcher.Watch(directoryAsset->info, filePath);
        }
        else if (extension == FY_INFO_EXTENSION)
        {
            AssetInfo* assetInfo = CreateAssetInfo();
            assetInfo->name = Path::Name(filePath);
            assetInfo->parent = parentDirectory->info;
            assetInfo->infoPath = filePath;
            assetInfo->absolutePath = Path::Join(Path::Parent(filePath), Path::Name(filePath), FY_ASSET_EXTENSION);
            assetInfo->payloadPath = assetInfo->absolutePath;
            assetInfo->imported = false;
            parentDirectory->info->AddChild(assetInfo);
            assetInfo->UpdatePath();

            LoadInfoJson(assetInfo->infoPath, assetInfo);
        }
        else if (auto importer = importers.Find(extension))
        {
            AssetIO* io = importer->second;

            AssetInfo* assetInfo = CreateAssetInfo();
            assetInfo->name = Path::Name(filePath);
            assetInfo->parent = parentDirectory->info;
            assetInfo->absolutePath = filePath;
            assetInfo->type = Registry::FindTypeById(io->getAssetTypeId(filePath));
            assetInfo->imported = true;
            parentDirectory->info->AddChild(assetInfo);
            assetInfo->UpdatePath();

            assetInfo->infoPath = Path::Join(Path::Parent(filePath), Path::Name(filePath), FY_IMPORT_EXTENSION);
            bool infoLoaded = LoadInfoJson(assetInfo->infoPath, assetInfo);

            if (!assetInfo->GetUUID())
            {
                assetInfo->SetUUID(UUID::RandomUUID());
            }

            String cacheDir = Path::Join(cacheDirectory, ToString(assetInfo->uuid));
            assetInfo->payloadPath = Path::Join(cacheDir, Path::Name(filePath), FY_ASSET_EXTENSION);

            bool payloadFound = FileSystem::GetFileStatus(assetInfo->payloadPath).exists;

            u64 lastModifiedTime = FileSystem::GetFileStatus(filePath).lastModifiedTime;

            if (!infoLoaded || !payloadFound || assetInfo->lastModifiedTime != lastModifiedTime)
            {
                assetInfo->lastModifiedTime = lastModifiedTime;
                QueueAssetImport(io, assetInfo);
            }

            fileWatcher.Watch(assetInfo, filePath);
        }
    }


    void AssetManager::QueueAssetImport(AssetIO* io, AssetInfo* assetInfo)
    {
        //TODO enqueue to a thread pool
        if (io->importAsset)
        {
            Asset* asset = LoadAsset(assetInfo);
            io->importAsset(assetInfo->absolutePath, asset);
            SaveInfoJson(assetInfo);
            SaveAssetsJson(assetInfo);
        }
    }

    void AssetManager::SaveInfoJson(AssetInfo* assetInfo)
    {
        logger.Debug("saving info {} on {} ", assetInfo->relativePath, assetInfo->infoPath);

        FY_ASSERT(!assetInfo->infoPath.Empty(), "infoPath path must be provided");
        JsonAssetWriter jsonAssetWriter;
        FileSystem::SaveFileAsString(assetInfo->infoPath, JsonAssetWriter::Stringify(SaveInfo(jsonAssetWriter, assetInfo)));
    }

    ArchiveObject AssetManager::SaveInfo(ArchiveWriter& writer, AssetInfo* assetInfo, bool isChild)
    {
        ArchiveObject object = writer.CreateObject();
        writer.WriteString(object, "uuid", ToString(assetInfo->GetUUID()));

        if (assetInfo->lastModifiedTime != 0)
        {
            writer.WriteUInt(object, "lastModifiedTime", assetInfo->lastModifiedTime);
        }

        writer.WriteString(object, "type", assetInfo->GetType()->GetName());

        if (isChild)
        {
            writer.WriteString(object, "name", assetInfo->GetName());
        }

        // if (ImportSettings* importSettings = asset->GetImportSettings(); importSettings != nullptr && importSettings->GetTypeHandler() != nullptr)
        // {
        //     writer.WriteValue(object, "importSettings", Serialization::Serialize(importSettings->GetTypeHandler(), writer, importSettings));
        // }

        if (!assetInfo->GetChildren().Empty())
        {
            ArchiveObject arr = writer.CreateArray();
            for (AssetInfo* subAsset : assetInfo->GetChildren())
            {
                writer.AddValue(arr, SaveInfo(writer, subAsset, true));
            }
            writer.WriteValue(object, "subAssets", arr);
        }
        return object;
    }

    void AssetManager::SaveAssetsJson(AssetInfo* assetInfo)
    {
        FY_ASSERT(!assetInfo->payloadPath.Empty(), "payload path must be provided");

        logger.Debug("saving asset {} on {} ", assetInfo->relativePath, assetInfo->payloadPath);

        JsonAssetWriter writer;
        Asset* asset = LoadAsset(assetInfo);
        ArchiveObject object = Serialization::Serialize(asset->GetInfo()->GetType(), writer, asset);
        FileSystem::SaveFileAsString(assetInfo->payloadPath, JsonAssetWriter::Stringify(object));

        for(AssetInfo* child: assetInfo->GetChildren())
        {
            SaveAssetsJson(child);
        }
    }

    // ArchiveObject AssetManager::SaveAsset(ArchiveWriter& writer, Asset* asset)
    // {
    //     ArchiveObject object = Serialization::Serialize(asset->GetInfo()->GetType(), writer, asset);
    //
    //     // if (root != asset)
    //     // {
    //     //     FY_ASSERT(asset->GetUUID(), "assets without uuid cannot be serialized");
    //     //     writer.WriteString(object, "_uuid", ToString(asset->GetUUID()));
    //     // }
    //
    //     // if (!asset->GetChildren().Empty())
    //     // {
    //     //     ArchiveObject arr = writer.CreateArray();
    //     //     for (Asset* child : asset->GetChildren())
    //     //     {
    //     //         ArchiveObject assetObj = serialize(writer, child);
    //     //         writer.AddValue(arr, assetObj);
    //     //     }
    //     //     writer.WriteValue(object, "_children", arr);
    //     // }
    //     return object;
    // }
    //
    // void AssetManager::SaveAssetJson(StringView file, Asset* asset)
    // {
    //     JsonAssetWriter writer;
    //     FileSystem::SaveFileAsString(file, JsonAssetWriter::Stringify(SaveAsset(writer, asset)));
    // }

    bool AssetManager::LoadInfoJson(StringView file, AssetInfo* assetInfo)
    {
        if (String str = FileSystem::ReadFileAsString(file); !str.Empty())
        {
            JsonAssetReader reader(str);
            LoadInfo(reader, reader.ReadObject(), assetInfo);
            return true;
        }
        return false;
    }

    void AssetManager::LoadInfo(ArchiveReader& reader, ArchiveObject object, AssetInfo* assetItem)
    {
        assetItem->lastModifiedTime = reader.ReadUInt(object, "lastModifiedTime");
        assetItem->type = Registry::FindTypeByName(reader.ReadString(object, "type"));
        assetItem->SetUUID(UUID::FromString(reader.ReadString(object, "uuid")));

        // if (ImportSettings* importSettings = asset->GetImportSettings())
        // {
        //     if (ArchiveObject settingsObj = reader.ReadObject(object, "importSettings"))
        //     {
        //         Serialization::Deserialize(importSettings->GetTypeHandler(), reader, settingsObj, importSettings);
        //     }
        // }
        //
        if (ArchiveObject arr = reader.ReadObject(object, "subAssets"))
        {
            auto size = reader.ArrSize(arr);

            ArchiveObject item{};
            for (usize i = 0; i < size; ++i)
            {
                item = reader.Next(arr, item);
                if (item)
                {
                    AssetInfo* child = CreateAssetInfo();
                    child->name = reader.ReadString(item, "name");
                    child->parent = assetItem;
                    child->infoPath = "";
                    child->payloadPath = ""; //TODO
                    child->imported = true;
                    assetItem->children.EmplaceBack(child);
                    LoadInfo(reader, item, child);
                }
            }
        }
    }

    void AssetManager::LoadAssetJson(StringView file, Asset* asset)
    {
        String content = FileSystem::ReadFileAsString(file);
        if (!content.Empty())
        {
            JsonAssetReader reader(FileSystem::ReadFileAsString(file));
            LoadAsset(reader, reader.ReadObject(), asset);
        }
    }

    void AssetManager::LoadAsset(ArchiveReader& reader, ArchiveObject object, Asset* asset)
    {
        Serialization::Deserialize(asset->GetInfo()->GetType(), reader, object, asset);

        // ArchiveObject arr = reader.ReadObject(object, "_children");
        // auto          size = reader.ArrSize(arr);
        // ArchiveObject item{};
        // for (usize i = 0; i < size; ++i)
        // {
        //     item = reader.Next(arr, item);
        //     if (item)
        //     {
        //         StringView uuidStr = reader.ReadString(item, "_uuid");
        //         if (Asset* childAsset = FindById(UUID::FromString(uuidStr)))
        //         {
        //             LoadAsset(reader, item, childAsset);
        //         }
        //         else
        //         {
        //             logger.Warn("child asset {} from {} not found", uuidStr, asset->GetPath());
        //         }
        //     }
        // }
    }

    void AssetManager::SaveOnDirectory(DirectoryAsset* directoryAsset, const StringView& directoryPath)
    {
        if (!FileSystem::GetFileStatus(directoryPath).exists)
        {
            FileSystem::CreateDirectory(directoryPath);
        }

        for (AssetInfo* asset : directoryAsset->info->GetChildren())
        {
            if (asset->IsModified())
            {
                SaveInfoJson(asset);
                SaveAssetsJson(asset);

                asset->persistedVersion = asset->currentVersion;
            }

            if (DirectoryAsset* childDirectory = dynamic_cast<DirectoryAsset*>(asset->GetInstance()))
            {
                SaveOnDirectory(childDirectory, asset->absolutePath);
            }
        }
    }

    void AssetManager::SetCacheDirectory(const StringView& directory)
    {
        cacheDirectory = directory;
        if (!FileSystem::GetFileStatus(cacheDirectory).exists)
        {
            FileSystem::CreateDirectory(cacheDirectory);
        }
    }

    StringView AssetManager::GetCacheDirectory()
    {
        FY_ASSERT(!cacheDirectory.Empty(), "cache dir not set");
        return cacheDirectory;
    }

    void AssetManager::GetUpdatedAssets(DirectoryAsset* directoryAsset, Array<AssetInfo*>& updatedAssets)
    {
        if (!directoryAsset) return;

        for (AssetInfo* asset : directoryAsset->GetInfo()->GetChildren())
        {
            if (asset->IsModified())
            {
                updatedAssets.EmplaceBack(asset);
            }

            if (DirectoryAsset* childDirectory = dynamic_cast<DirectoryAsset*>(asset->GetInstance()))
            {
                GetUpdatedAssets(childDirectory, updatedAssets);
            }
        }
    }


    DirectoryAsset* AssetManager::LoadFromPackage(const StringView& name, const StringView& file, const StringView& binFile)
    {
        FY_ASSERT(false, "not implemented");
        return nullptr;
    }

    void AssetManager::ImportAsset(DirectoryAsset* directory, const StringView& path)
    {
        FileSystem::CopyFile(path, Path::Join(directory->GetInfo()->GetAbsolutePath(), Path::Name(path), Path::Extension(path)));
        fileWatcher.Check();
    }

    bool AssetManager::CanReimportAsset(AssetInfo* assetInfo)
    {
        return importers.Find(assetInfo->GetExtension()) != importers.end();
    }

    void AssetManager::ReimportAsset(AssetInfo* asset)
    {
        if (auto importer = importers.Find(asset->GetExtension()))
        {
            QueueAssetImport(importer->second, asset);
        }
    }

    void AssetManager::DestroyAssets()
    {
        if (Engine::IsRunning())
        {
            logger.Critical("DestroyAssets() cannot be called when the engine is running");
            return;
        }

        for (AssetInfo* assetInfo : assets)
        {
            UnloadAsset(assetInfo);
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(assetInfo);
        }

        assets.Clear();
        assets.ShrinkToFit();

        assetsById.Clear();
        assetsByPath.Clear();
    }

    void AssetManager::OnUpdate(f64 deltaTime)
    {
        fileWatcher.CheckForUpdates([](const FileWatcherModified& modified)
        {
            if (Path::Extension(modified.path) == FY_INFO_EXTENSION)
            {
                return;
            }

            AssetInfo* assetInfo = static_cast<AssetInfo*>(modified.userData);

            switch (modified.event)
            {
                case FileNotifyEvent::Added:
                {
                    logger.Debug("FileWatcher FileNotifyEvent::Added {} ", modified.path);
                    if (DirectoryAsset* directory = dynamic_cast<DirectoryAsset*>(assetInfo->GetInstance()); assetInfo->FindChildByAbsolutePath(modified.path) == nullptr)
                    {
                        LoadAssetFile(directory, modified.path);
                    }
                    break;
                }
                case FileNotifyEvent::Removed:
                    logger.Debug("FileWatcher FileNotifyEvent::Removed {} ", modified.path);
                    assetInfo->Delete();
                    break;
                case FileNotifyEvent::Modified:
                    logger.Debug("FileWatcher FileNotifyEvent::Modified {} ", modified.path);
                    ReimportAsset(assetInfo);
                    break;
                case FileNotifyEvent::Renamed:
                    logger.Debug("FileWatcher FileNotifyEvent::Renamed {} ", modified.path);
                    assetInfo->SetName(modified.name);
                    break;
            }
        });
    }

    AssetInfo* AssetManager::CreateAssetInfo()
    {
        AssetInfo* assetInfo = MemoryGlobals::GetDefaultAllocator().Alloc<AssetInfo>();
        assets.EmplaceBack(assetInfo);
        return assetInfo;
    }

    Asset* AssetManager::LoadAsset(AssetInfo* assetInfo)
    {
        if (assetInfo->instance == nullptr && assetInfo->type != nullptr)
        {
            assetInfo->instance = assetInfo->type->Cast<Asset>(assetInfo->type->NewInstance());
            assetInfo->instance->info = assetInfo;
            LoadAssetJson(assetInfo->payloadPath, assetInfo->instance);
        }
        return assetInfo->instance;
    }

    void AssetManager::UnloadAsset(AssetInfo* assetInfo)
    {
        if (assetInfo->instance != nullptr && assetInfo->type != nullptr)
        {
            assetInfo->type->Destroy(assetInfo->instance);
            assetInfo->instance = nullptr;
        }
    }

    void AssetDatabaseInit()
    {
        if (hotReloadEnabled)
        {
            fileWatcher.Start();
        }
        Event::Bind<OnUpdate, AssetManager::OnUpdate>();
    }

    void AssetManager::EnableHotReload(bool enable)
    {
        hotReloadEnabled = enable;
    }

    void AssetManager::WatchAsset(AssetInfo* assetInfo)
    {
        if (assetInfo && hotReloadEnabled)
        {
            fileWatcher.Watch(assetInfo, assetInfo->GetAbsolutePath());
        }
    }

    void AssetDatabaseShutdown()
    {
        Event::Unbind<OnUpdate, AssetManager::OnUpdate>();

        hotReloadEnabled = false;
        fileWatcher.Stop();

        AssetManager::DestroyAssets();

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
