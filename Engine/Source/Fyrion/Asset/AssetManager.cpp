#include "AssetManager.hpp"
#include <memory>

#include "AssetHandler.hpp"
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
        Array<AssetHandler*>                  assets;
        HashMap<UUID, AssetHandler*>          assetsById;
        HashMap<String, AssetHandler*>        assetsByPath;
        Array<Pair<TypeID, AssetIO*>>      assetIOs;
        HashMap<String, AssetIO*>          importers;
        HashMap<TypeID, Array<AssetHandler*>> assetsByType;
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

    void AssetDatabaseCleanRefs(AssetHandler* assetInfo)
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

    void AssetManagerUpdatePath(AssetHandler* assetInfo, const StringView& oldPath, const StringView& newPath)
    {
        if (!oldPath.Empty())
        {
            assetsByPath.Erase(oldPath);
        }
        assetsByPath.Insert(String{newPath}, assetInfo);
        logger.Debug("asset {} registred to path {} ", assetInfo->GetName(), newPath);
    }

    void AssetManagerUpdateUUID(AssetHandler* assetInfo, const UUID& newUUID)
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

    AssetHandler* AssetManager::FindHandlerByPath(const StringView& path)
    {
        return nullptr;
    }

    Asset* AssetManager::Create(TypeHandler* typeHandler, const AssetCreation& assetCreation)
    {
       return nullptr;
    }

    DirectoryAssetHandler* AssetManager::LoadFromDirectory(const StringView& name, const StringView& directory)
    {
        return nullptr;
    }

    void AssetManager::LoadAssetFile(DirectoryAssetHandler* parentDirectory, const StringView& filePath)
    {

    }


    void AssetManager::QueueAssetImport(AssetIO* io, AssetHandler* assetInfo)
    {
        //TODO enqueue to a thread pool
        if (io->importAsset)
        {
            logger.Debug("Importing file {} ", assetInfo->GetAbsolutePath());
            io->importAsset(assetInfo->GetAbsolutePath(), assetInfo->LoadInstance());
            assetInfo->Save();
        }
    }

    void AssetManager::SaveOnDirectory(DirectoryAssetHandler* directoryAssetHandler, const StringView& directoryPath)
    {
        if (!FileSystem::GetFileStatus(directoryPath).exists)
        {
            FileSystem::CreateDirectory(directoryPath);
        }

        for (AssetHandler* assetInfo : directoryAssetHandler->GetChildren())
        {
            if (assetInfo->IsModified())
            {
                assetInfo->Save();
            }

            if (DirectoryAssetHandler* childDirectory = dynamic_cast<DirectoryAssetHandler*>(assetInfo))
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

    void AssetManager::GetUpdatedAssets(DirectoryAssetHandler* directoryAssetHandler, Array<AssetHandler*>& updatedAssets)
    {
        if (!directoryAssetHandler) return;

        for (AssetHandler* child : directoryAssetHandler->GetChildren())
        {
            if (child->IsModified())
            {
                updatedAssets.EmplaceBack(child);
            }

            if (DirectoryAssetHandler* childDirectory = dynamic_cast<DirectoryAssetHandler*>(child))
            {
                GetUpdatedAssets(childDirectory, updatedAssets);
            }
        }
    }


    DirectoryAssetHandler* AssetManager::LoadFromPackage(const StringView& name, const StringView& file, const StringView& binFile)
    {
        FY_ASSERT(false, "not implemented");
        return nullptr;
    }

    void AssetManager::ImportAsset(DirectoryAssetHandler* directory, const StringView& path)
    {
        FileSystem::CopyFile(path, Path::Join(directory->GetAbsolutePath(), Path::Name(path), Path::Extension(path)));
        fileWatcher.Check();
    }

    bool AssetManager::CanReimportAsset(AssetHandler* assetInfo)
    {
        return importers.Find(assetInfo->GetExtension()) != importers.end();
    }

    void AssetManager::ReimportAsset(AssetHandler* asset)
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

        for (AssetHandler* assetInfo : assets)
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

            AssetHandler* assetInfo = static_cast<AssetHandler*>(modified.userData);

            switch (modified.event)
            {
                case FileNotifyEvent::Added:
                {
                    logger.Debug("FileWatcher FileNotifyEvent::Added {} ", modified.path);
                    if (DirectoryAssetHandler* directory = dynamic_cast<DirectoryAssetHandler*>(assetInfo); assetInfo->FindChildByAbsolutePath(modified.path) == nullptr)
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

    void AssetManager::WatchAsset(AssetHandler* assetInfo)
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
