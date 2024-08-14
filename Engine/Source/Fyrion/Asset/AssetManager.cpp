#include "AssetManager.hpp"
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
            return it->second->LoadInstance();
        }
        return nullptr;
    }

    Asset* AssetManager::LoadByPath(const StringView& path)
    {
        if (auto it = assetsByPath.Find(path))
        {
            return it->second->LoadInstance();
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
   //     assetInfo->absolutePath = assetCreation.absolutePath;
        assetInfo->type = typeHandler;
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
           // assetInfo->absolutePath = Path::Join(assetInfo->parent->GetAbsolutePath(), assetInfo->name, typeId != GetTypeID<DirectoryAsset>() ? FY_ASSET_EXTENSION : "");
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

        Asset* asset = assetInfo->LoadInstance();
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

            for (const auto& entry : DirectoryEntries{filePath})
            {
                LoadAssetFile(directoryAsset, entry);
            }

            fileWatcher.Watch(directoryAsset->info, filePath);
        }
        else if (extension == FY_INFO_EXTENSION)
        {
            AssetInfoJson* assetInfo = CreateAssetInfoJson(parentDirectory->info, filePath);
            assetInfo->assetPath = Path::Join(Path::Parent(filePath), Path::Name(filePath), FY_ASSET_EXTENSION);
            assetInfo->absolutePath = assetInfo->assetPath;
            assetInfo->persistedVersion = assetInfo->currentVersion;
            assetInfo->imported = false;

            assetInfo->UpdatePath();
        }
        else if (auto importer = importers.Find(extension))
        {
            AssetIO* io = importer->second;

            AssetInfoJson* assetInfo = CreateAssetInfoJson(parentDirectory->info, Path::Join(Path::Parent(filePath), Path::Name(filePath), FY_IMPORT_EXTENSION));
            assetInfo->assetPath = Path::Join(GetCacheDirectory(), ToString(assetInfo->uuid), ToString(assetInfo->name), FY_ASSET_EXTENSION);
            assetInfo->importedFilePath = filePath;
            assetInfo->absolutePath = filePath;
            assetInfo->imported = true;
            assetInfo->type = Registry::FindTypeById(io->getAssetTypeId(filePath));
            assetInfo->persistedVersion = assetInfo->currentVersion;

            assetInfo->UpdatePath();

            u64 lastModifiedTime = FileSystem::GetFileStatus(filePath).lastModifiedTime;

            if (!assetInfo->infoLoaded || assetInfo->lastModifiedTime != lastModifiedTime || !FileSystem::GetFileStatus(assetInfo->assetPath).exists)
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
            io->importAsset(assetInfo->GetAbsolutePath(), assetInfo->LoadInstance());
            assetInfo->Save();
        }
    }

    void AssetManager::SaveOnDirectory(DirectoryAsset* directoryAsset, const StringView& directoryPath)
    {
        if (!FileSystem::GetFileStatus(directoryPath).exists)
        {
            FileSystem::CreateDirectory(directoryPath);
        }

        for (AssetInfo* assetInfo : directoryAsset->info->GetChildren())
        {
            if (assetInfo->IsModified())
            {
                String infoPath = Path::Join(Path::Parent(directoryPath), assetInfo->name, FY_INFO_EXTENSION);
                String assetPath = Path::Join(Path::Parent(directoryPath), assetInfo->name, FY_ASSET_EXTENSION);

                JsonAssetWriter writer;
                FileSystem::SaveFileAsString(infoPath, JsonAssetWriter::Stringify(assetInfo->Serialize(writer)));

              //  assetInfo->persistedVersion = assetInfo->currentVersion;

                Asset* asset = assetInfo->LoadInstance();

                //asset->Seria

                //TODO save asset children on .fy_data
            }

            if (DirectoryAsset* childDirectory = dynamic_cast<DirectoryAsset*>(assetInfo->GetInstance()))
            {
                SaveOnDirectory(childDirectory, assetInfo->GetAbsolutePath());
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
            assetInfo->UnloadInstance();
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

    AssetInfoJson* AssetManager::CreateAssetInfoJson(AssetInfo* parent, StringView infoPath)
    {
        AssetInfoJson* assetInfoJson = MemoryGlobals::GetDefaultAllocator().Alloc<AssetInfoJson>();
        assetInfoJson->name = Path::Name(infoPath);
        assetInfoJson->parent = parent;
        assetInfoJson->infoPath = infoPath;
        assetInfoJson->currentVersion = 1;

        if (parent)
        {
            parent->AddChild(assetInfoJson);
        }

        assets.EmplaceBack(assetInfoJson);

        if (const String str = FileSystem::ReadFileAsString(infoPath); !str.Empty())
        {
            JsonAssetReader reader(str);
            assetInfoJson->Deserialize(reader, reader.ReadObject());
            assetInfoJson->infoLoaded = true;
        }

        if (!assetInfoJson->GetUUID())
        {
            assetInfoJson->SetUUID(UUID::RandomUUID());
        }

        return assetInfoJson;
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
