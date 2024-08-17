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

        String                                dataDirectory;
        Array<AssetHandler*>                  assets;
        HashMap<UUID, AssetHandler*>          assetsById;
        HashMap<String, AssetHandler*>        assetsByPath;
        Array<Pair<TypeID, AssetIO*>>         assetIOs;
        HashMap<String, AssetIO*>             importers;
        HashMap<TypeID, Array<AssetHandler*>> assetsByType;
        FileWatcher                           fileWatcher;
        bool                                  hotReloadEnabled = false;

        Logger& logger = Logger::GetLogger("Fyrion::AssetManager");

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

    void AssetManagerAddHandler(AssetHandler* assetHandler)
    {
        assets.EmplaceBack(assetHandler);
    }

    void AssetManagerUpdateType(AssetHandler* assetHandler, TypeHandler* typeHandler)
    {
        auto itByType = assetsByType.Find(typeHandler->GetTypeInfo().typeId);
        if (itByType == assetsByType.end())
        {
            itByType = assetsByType.Emplace(typeHandler->GetTypeInfo().typeId, {}).first;
        }
        itByType->second.EmplaceBack(assetHandler);
    }

    void AssetManagerCleanRefs(AssetHandler* assetHandler)
    {
        assetsById.Erase(assetHandler->GetUUID());
        assetsByPath.Erase(assetHandler->GetPath());

        if(assetHandler->GetType() != nullptr)
        {
            if (auto it = assetsByType.Find(assetHandler->GetType()->GetTypeInfo().typeId))
            {
                if (const auto itArr = FindFirst(it->second.begin(), it->second.end(), assetHandler))
                {
                    it->second.Erase(itArr);
                }
            }
        }
    }

    void AssetManagerUpdatePath(AssetHandler* assetHandler, const StringView& oldPath, const StringView& newPath)
    {
        if (!oldPath.Empty())
        {
            assetsByPath.Erase(oldPath);
        }
        assetsByPath.Insert(String{newPath}, assetHandler);
        logger.Debug("asset {} registred to path {} ", assetHandler->GetName(), newPath);
    }

    void AssetManagerUpdateUUID(AssetHandler* assetHandler, const UUID& newUUID)
    {
        if (assetHandler->GetUUID())
        {
            assetsById.Erase(assetHandler->GetUUID());
        }

        if (newUUID)
        {
            assetsById.Insert(newUUID, assetHandler);
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

    Span<AssetHandler*> AssetManager::FindAssetsByType(TypeID typeId)
    {
        if (auto it = assetsByType.Find(typeId))
        {
            return it->second;
        }
        return {};
    }

    AssetHandler* AssetManager::FindHandlerByPath(const StringView& path)
    {
        if (auto it = assetsByPath.Find(path))
        {
            return it->second;
        }
        return nullptr;
    }

    DirectoryAssetHandler* AssetManager::CreateDirectory(DirectoryAssetHandler* parent, StringView name)
    {
        DirectoryAssetHandler* handler = DirectoryAssetHandler::Create(name, Path::Join(parent->GetAbsolutePath(), name), parent);
        handler->Save();
        fileWatcher.Watch(handler, handler->GetAbsolutePath());
        logger.Debug("directory {} created on {}", handler->GetName(), handler->GetAbsolutePath());
        return handler;
    }

    Asset* AssetManager::Create(TypeHandler* typeHandler, const AssetCreation& assetCreation)
    {
        String name = !assetCreation.name.Empty() ? String(assetCreation.name) : String("New ").Append(AssetHandler::GetDisplayName(typeHandler));

        if (assetCreation.directoryAsset != nullptr)
        {
            JsonAssetHandler* handler = JsonAssetHandler::Create(AssetHandler::ValidateName(assetCreation.directoryAsset, nullptr, name),
                                                                 assetCreation.directoryAsset);
            handler->SetUUID(UUID::RandomUUID());
            handler->SetType(typeHandler);

            return handler->LoadInstance();
        }

        if (assetCreation.parent != nullptr)
        {
            AssetHandler* handler = assetCreation.parent->CreateChild(name);
            handler->SetType(typeHandler);
            handler->SetUUID(UUID::RandomUUID());
            logger.Debug("asset {} created type {} ", name, typeHandler->GetName());
            return handler->LoadInstance();
        }

        logger.Critical("asset cannot be created");

        return nullptr;
    }

    DirectoryAssetHandler* AssetManager::LoadFromDirectory(const StringView& name, const StringView& directory)
    {
        if (!FileSystem::GetFileStatus(directory).exists)
        {
            return nullptr;
        }

        DirectoryAssetHandler* handler = DirectoryAssetHandler::Create(name, directory, nullptr);

        for (const auto& entry : DirectoryEntries{directory})
        {
            LoadAssetFile(handler, entry);
        }

        fileWatcher.Watch(handler, directory);
        return handler;
    }

    void AssetManager::LoadAssetFile(DirectoryAssetHandler* parentDirectory, const StringView& filePath)
    {
        String extension = Path::Extension(filePath);
        if (extension == FY_DATA_EXTENSION) return;

        if (FileSystem::GetFileStatus(filePath).isDirectory)
        {
            DirectoryAssetHandler* handler = DirectoryAssetHandler::Create(
                Path::Name(filePath),
                filePath,
                parentDirectory);

            for (const auto& entry : DirectoryEntries{filePath})
            {
                LoadAssetFile(handler, entry);
            }

            fileWatcher.Watch(handler, filePath);
        }
        else if (extension == FY_INFO_EXTENSION)
        {
            JsonAssetHandler::Create(Path::Name(filePath), parentDirectory);
        }
        else if (auto importer = importers.Find(extension))
        {
            AssetIO* io = importer->second;
            ImportedAssetHandler* handler = ImportedAssetHandler::Create(io, filePath, parentDirectory);
            fileWatcher.Watch(handler, filePath);
        }
    }


    void AssetManager::QueueAssetImport(AssetIO* io, AssetHandler* assetHandler)
    {
        //TODO enqueue to a thread pool
        if (io->importAsset)
        {
            logger.Debug("Importing file {} ", assetHandler->GetAbsolutePath());
            io->importAsset(assetHandler->GetAbsolutePath(), assetHandler->LoadInstance());
            assetHandler->Save();
        }
    }

    void AssetManager::SaveOnDirectory(DirectoryAssetHandler* directoryAssetHandler, const StringView& directoryPath)
    {
        if (!FileSystem::GetFileStatus(directoryPath).exists)
        {
            FileSystem::CreateDirectory(directoryPath);
        }

        for (AssetHandler* assetHandler : directoryAssetHandler->GetChildren())
        {
            if (assetHandler->IsModified())
            {
                assetHandler->Save();
            }

            if (DirectoryAssetHandler* childDirectory = dynamic_cast<DirectoryAssetHandler*>(assetHandler))
            {
                SaveOnDirectory(childDirectory, assetHandler->GetAbsolutePath());
            }
        }
    }

    void AssetManager::SetDataDirectory(const StringView& directory)
    {
        dataDirectory = directory;
        if (!FileSystem::GetFileStatus(dataDirectory).exists)
        {
            FileSystem::CreateDirectory(dataDirectory);
        }
    }

    StringView AssetManager::GetDataDirectory()
    {
        FY_ASSERT(!dataDirectory.Empty(), "data dir not set");
        return dataDirectory;
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

    bool AssetManager::CanReimportAsset(AssetHandler* assetHandler)
    {
        return importers.Find(assetHandler->GetExtension()) != importers.end();
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

        for (AssetHandler* assetHandler : assets)
        {
            assetHandler->UnloadInstance();
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(assetHandler);
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

            AssetHandler* assetHandler = static_cast<AssetHandler*>(modified.userData);

            switch (modified.event)
            {
                case FileNotifyEvent::Added:
                {
                    logger.Debug("FileWatcher FileNotifyEvent::Added {} ", modified.path);
                    if (DirectoryAssetHandler* directory = dynamic_cast<DirectoryAssetHandler*>(assetHandler); assetHandler->FindChildByAbsolutePath(modified.path) == nullptr)
                    {
                        LoadAssetFile(directory, modified.path);
                    }
                    break;
                }
                case FileNotifyEvent::Removed:
                    logger.Debug("FileWatcher FileNotifyEvent::Removed {} ", modified.path);
                    if (modified.path == assetHandler->GetAbsolutePath())
                    {
                        assetHandler->Delete();
                    }
                    break;
                case FileNotifyEvent::Modified:
                    logger.Debug("FileWatcher FileNotifyEvent::Modified {} ", modified.path);
                    ReimportAsset(assetHandler);
                    break;
                case FileNotifyEvent::Renamed:
                    logger.Debug("FileWatcher FileNotifyEvent::Renamed {} ", modified.path);
                    if (modified.name != assetHandler->GetName())
                    {
                        assetHandler->SetName(modified.name);
                    }
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

    void AssetManager::WatchAsset(AssetHandler* assetHandler)
    {
        if (assetHandler && hotReloadEnabled)
        {
            fileWatcher.Watch(assetHandler, assetHandler->GetAbsolutePath());
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

        dataDirectory.Clear();

        assetIOs.Clear();
        assetIOs.ShrinkToFit();
        importers.Clear();
    }
}
