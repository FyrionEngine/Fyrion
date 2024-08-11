#include "AssetManager.hpp"

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

        String                             cacheDirectory;
        Array<Asset*>                      assets;
        HashMap<UUID, AssetInfo*>          assetsById;
        HashMap<String, AssetInfo*>        assetsByPath;
        Array<Pair<TypeID, AssetIO*>>      assetIOs;
        HashMap<String, AssetIO*>          importers;
        HashMap<TypeID, Array<AssetInfo*>> assetsByType;
        FileWatcher                        fileWatcher;
        bool                               hotReloadEnabled = false;

        Logger& logger = Logger::GetLogger("Fyrion::AssetDatabase");
        bool    registerEvents = RegisterEvents();

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


    Asset* AssetManager::FindById(const UUID& assetId)
    {
        // if (!assetId) return nullptr;
        //
        // if (auto it = assetsById.Find(assetId))
        // {
        //     return it->second;
        // }

        return nullptr;
    }

    Asset* AssetManager::FindByPath(const StringView& path)
    {
        // if (auto it = assetsByPath.Find(path))
        // {
        //     return it->second;
        // }

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
#if 0
        FY_ASSERT(typeHandler, "type not found");
        if (!typeHandler) return nullptr;

        bool newInstance = false;

        Asset* asset = nullptr;
        if (auto it = assetsById.Find(assetCreation.uuid))
        {
            asset = it->second;
        }
        else if (auto it = assetsByPath.Find(assetCreation.desiredPath))
        {
            asset = it->second;
        }
        else
        {
            asset = typeHandler->Cast<Asset>(typeHandler->NewInstance());
            FY_ASSERT(asset, "type cannot be casted to Asset, check if FY_BASE_TYPES(Asset) is included on the class");
            newInstance = true;
        }

        if (!asset)
        {
            return nullptr;
        }

        TypeID typeId = typeHandler->GetTypeInfo().typeId;

        if (asset->parent == nullptr && assetCreation.parent != nullptr)
        {
            assetCreation.parent->AddChild(asset);
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
            asset->absolutePath = Path::Join(asset->parent->GetAbsolutePath(), asset->name, typeId != GetTypeID<DirectoryInfo>() ? FY_ASSET_EXTENSION : "");
        }

        if (newInstance)
        {
            assets.EmplaceBack(asset);

            auto itByType = assetsByType.Find(typeId);
            if (itByType == assetsByType.end())
            {
                itByType = assetsByType.Emplace(typeId, {}).first;
            }
            itByType->second.EmplaceBack(asset);
        }

        if (asset->uuid)
        {
            assetsById.Insert(asset->uuid, asset);
        }

        if (!assetCreation.loading)
        {
            asset->OnCreated();
        }

        return asset;
#endif
        return nullptr;
    }

    DirectoryInfo* AssetManager::LoadFromDirectory(const StringView& name, const StringView& directory)
    {
        if (!FileSystem::GetFileStatus(directory).exists)
        {
            return nullptr;
        }

        DirectoryInfo* directoryInfo = MemoryGlobals::GetDefaultAllocator().Alloc<DirectoryInfo>();
        directoryInfo->name = name;
        directoryInfo->path = String(name) + ":/";
        directoryInfo->absolutePath = directory;

        for (const auto& entry : DirectoryEntries{directory})
        {
            LoadAssetFile(directoryInfo, entry);
        }

        return directoryInfo;

#if 0
        if (!FileSystem::GetFileStatus(directory).exists)
        {
            return nullptr;
        }

        AssetDirectory* assetDirectory = Create<AssetDirectory>(AssetCreation{
            .name = name,
            .desiredPath = String(name) + ":/",
            .absolutePath = directory,
            .loading = true,
        });

        assetsByPath.Insert(assetDirectory->GetPath(), assetDirectory);

        for (const auto& entry : DirectoryEntries{directory})
        {
            LoadAssetFile(assetDirectory, entry);
        }

        fileWatcher.Watch(assetDirectory, directory);

        return assetDirectory;
#endif
    }

    void AssetManager::LoadAssetFile(DirectoryInfo* parentDirectory, const StringView& filePath)
    {
#if 0
        String extension = Path::Extension(filePath);
        if (FileSystem::GetFileStatus(filePath).isDirectory)
        {
            DirectoryMetaInfo* assetDirectory = Create<DirectoryMetaInfo>({
                .name = Path::Name(filePath),
                .parent = parentDirectory,
                .absolutePath = filePath,
                .loading = true,
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
            JsonAssetReader reader(FileSystem::ReadFileAsString(filePath));
            ArchiveObject object = reader.ReadObject();
            if (TypeHandler* typeHandler = Registry::FindTypeByName(reader.ReadString(object, "type")))
            {
                String assetPath = Path::Join(Path::Parent(filePath), Path::Name(filePath), FY_ASSET_EXTENSION);

                Asset* asset = Create(typeHandler, AssetCreation{
                                          .name = Path::Name(filePath),
                                          .parent = parentDirectory,
                                          .absolutePath = assetPath,
                                          .generateUUID = false,
                                          .loading = true,
                                      });

                LoadInfo(reader, object , asset);
                LoadAssetJson(assetPath, asset);

                asset->loadedVersion = asset->currentVersion;

                fileWatcher.Watch(asset, filePath);
            }
        }
        else if (auto importer = importers.Find(extension))
        {
            AssetIO* io = importer->second;

            Asset* asset = Create(Registry::FindTypeById(io->getAssetTypeId(filePath)), AssetCreation{
                                      .name = Path::Name(filePath),
                                      .parent = parentDirectory,
                                      .absolutePath = filePath,
                                      .generateUUID = false,
                                      .loading = true,
                                  });

            asset->imported = true;

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
                bool cacheExists = FileSystem::GetFileStatus(asset->GetCacheDirectory()).exists;

                if (cacheExists)
                {
                    String assetPath = Path::Join(asset->GetCacheDirectory(), asset->name, FY_ASSET_EXTENSION);
                    LoadAssetJson(assetPath, asset);
                }

                if (u64 lastModifiedTime = FileSystem::GetFileStatus(filePath).lastModifiedTime; asset->lastModifiedTime != lastModifiedTime || !cacheExists)
                {
                    asset->lastModifiedTime = lastModifiedTime;
                    QueueAssetImport(io, asset);
                }
            }
            fileWatcher.Watch(asset, filePath);
        }
#endif
    }


    void AssetManager::QueueAssetImport(AssetIO* io, Asset* asset)
    {
        //TODO enqueue to a thread pool
        if (io->importAsset)
        {
            // io->importAsset(asset->absolutePath, asset);
            //
            // String assetPath = Path::Join(asset->GetCacheDirectory(), asset->name, FY_ASSET_EXTENSION);
            // String infoPath = Path::Join(asset->GetParent()->GetAbsolutePath(), asset->name, FY_IMPORT_EXTENSION);
            // SaveInfoJson(infoPath, asset);
            // SaveAssetJson(assetPath, asset);
        }
    }

    void AssetManager::SaveInfoJson(StringView file, Asset* asset)
    {
        JsonAssetWriter jsonAssetWriter;
        FileSystem::SaveFileAsString(file, JsonAssetWriter::Stringify(SaveInfo(jsonAssetWriter, asset)));
    }

    ArchiveObject AssetManager::SaveInfo(ArchiveWriter& writer, Asset* asset, bool isChild)
    {
        // ArchiveObject object = writer.CreateObject();
        // writer.WriteString(object, "uuid", ToString(asset->GetUUID()));
        // if (asset->lastModifiedTime != 0)
        // {
        //     writer.WriteUInt(object, "lastModifiedTime", asset->lastModifiedTime);
        // }
        // writer.WriteString(object, "type", asset->GetType()->GetName());
        // if (isChild)
        // {
        //     writer.WriteString(object, "name", asset->GetName());
        // }
        // if (ImportSettings* importSettings = asset->GetImportSettings(); importSettings != nullptr && importSettings->GetTypeHandler() != nullptr)
        // {
        //     writer.WriteValue(object, "importSettings", Serialization::Serialize(importSettings->GetTypeHandler(), writer, importSettings));
        // }
        // if (!asset->GetChildren().Empty())
        // {
        //     ArchiveObject arr = writer.CreateArray();
        //     for (Asset* child : asset->GetChildren())
        //     {
        //         writer.AddValue(arr, SaveInfo(writer, child, true));
        //     }
        //     writer.WriteValue(object, "children", arr);
        // }
        // return object;

        return {};
    }

    void AssetManager::SaveAssetJson(StringView file, Asset* root)
    {
        // JsonAssetWriter                                                   writer;
        // std::function<ArchiveObject(ArchiveWriter& writer, Asset* asset)> serialize;
        // serialize = [&](ArchiveWriter& writer, Asset* asset)
        // {
        //     ArchiveObject object = Serialization::Serialize(asset->GetType(), writer, asset);
        //     if (root != asset)
        //     {
        //         FY_ASSERT(asset->GetUUID(), "assets without uuid cannot be serialized");
        //         writer.WriteString(object, "_uuid", ToString(asset->GetUUID()));
        //     }
        //
        //     if (!asset->GetChildren().Empty())
        //     {
        //         ArchiveObject arr = writer.CreateArray();
        //         for (Asset* child : asset->GetChildren())
        //         {
        //             ArchiveObject assetObj = serialize(writer, child);
        //             writer.AddValue(arr, assetObj);
        //         }
        //         writer.WriteValue(object, "_children", arr);
        //     }
        //     return object;
        // };
        // FileSystem::SaveFileAsString(file, JsonAssetWriter::Stringify(serialize(writer, root)));
    }

    void AssetManager::LoadInfoJson(StringView file, Asset* asset)
    {
        JsonAssetReader reader(FileSystem::ReadFileAsString(file));
        LoadInfo(reader, reader.ReadObject(), asset);
    }

    void AssetManager::LoadInfo(ArchiveReader& reader, ArchiveObject object, Asset* asset)
    {
        // asset->lastModifiedTime = reader.ReadUInt(object, "lastModifiedTime");
        // asset->SetUUID(UUID::FromString(reader.ReadString(object, "uuid")));
        //
        // if (ImportSettings* importSettings = asset->GetImportSettings())
        // {
        //     if (ArchiveObject settingsObj = reader.ReadObject(object, "importSettings"))
        //     {
        //         Serialization::Deserialize(importSettings->GetTypeHandler(), reader, settingsObj, importSettings);
        //     }
        // }
        //
        // if (ArchiveObject arr = reader.ReadObject(object, "children"))
        // {
        //     auto          size = reader.ArrSize(arr);
        //     ArchiveObject item{};
        //     for (usize i = 0; i < size; ++i)
        //     {
        //         item = reader.Next(arr, item);
        //         if (item)
        //         {
        //             if (TypeHandler* typeHandler = Registry::FindTypeByName(reader.ReadString(item, "type")))
        //             {
        //                 Asset* child = Create(typeHandler, AssetCreation{
        //                                           .name = reader.ReadString(item, "name"),
        //                                           .parent = asset,
        //                                           .generateUUID = false,
        //                                       });
        //                 LoadInfo(reader, item, child);
        //             }
        //         }
        //     }
        // }
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
        // Serialization::Deserialize(asset->GetType(), reader, object, asset);
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

    void AssetManager::SaveOnDirectory(DirectoryInfo* directoryInfo, const StringView& directoryPath)
    {
        if (!FileSystem::GetFileStatus(directoryPath).exists)
        {
            FileSystem::CreateDirectory(directoryPath);
        }

        // for (AssetInfo* asset : directoryInfo->GetChildren())
        // {
        //     if (asset->IsModified())
        //     {
        //         String infoPath = Path::Join(Path::Parent(asset->GetAbsolutePath()), asset->GetName(), FY_INFO_EXTENSION);
        //         String assetPath = Path::Join(Path::Parent(asset->GetAbsolutePath()), asset->GetName(), FY_ASSET_EXTENSION);
        //
        //         bool newFile = !FileSystem::GetFileStatus(assetPath).exists;
        //
        //         SaveInfoJson(infoPath, asset);
        //         SaveAssetJson(assetPath, asset);
        //
        //         asset->loadedVersion = asset->currentVersion;
        //
        //         if (newFile)
        //         {
        //             fileWatcher.Watch(asset, assetPath);
        //         }
        //     }
        // }
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

    void AssetManager::GetUpdatedAssets(DirectoryInfo* directoryInfo, Array<AssetInfo*>& updatedAssets)
    {
        if (!directoryInfo) return;

        // for (Asset* asset : directoryInfo->GetChildren())
        // {
        //     if (asset->IsModified())
        //     {
        //         updatedAssets.EmplaceBack(asset);
        //     }
        //
        //     if (DirectoryInfo* childDirectory = dynamic_cast<DirectoryInfo*>(asset))
        //     {
        //         GetUpdatedAssets(childDirectory, updatedAssets);
        //     }
        // }
    }


    DirectoryInfo* AssetManager::LoadFromPackage(const StringView& name, const StringView& file, const StringView& binFile)
    {
        FY_ASSERT(false, "not implemented");
        return nullptr;
    }

    void AssetManager::ImportAsset(DirectoryInfo* directory, const StringView& path)
    {
        FileSystem::CopyFile(path, Path::Join(directory->GetAbsolutePath(), Path::Name(path), Path::Extension(path)));
        fileWatcher.Check();
    }

    bool AssetManager::CanReimportAsset(AssetInfo* assetInfo)
    {
        return importers.Find(assetInfo->GetExtension()) != importers.end();
    }

    void AssetManager::ReimportAsset(AssetInfo* asset)
    {
        // if (auto importer = importers.Find(asset->GetExtension()))
        // {
        //     QueueAssetImport(importer->second, asset);
        // }
    }

    void AssetManager::DestroyAssets()
    {
        if (Engine::IsRunning())
        {
            logger.Critical("DestroyAssets() cannot be called when the engine is running");
            return;
        }

        // for (Asset* asset : assets)
        // {
        //     asset->GetType()->Destroy(asset);
        // }

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

            switch (modified.event)
            {
                case FileNotifyEvent::Added:
                {
                    if (DirectoryInfo* directory = static_cast<DirectoryInfo*>(modified.userData); directory->FindChildByAbsolutePath(modified.path) == nullptr)
                    {
                        LoadAssetFile(directory, modified.path);
                    }
                    break;
                }
                case FileNotifyEvent::Removed:
                    static_cast<Asset*>(modified.userData)->GetInfo()->Destroy();
                    break;
                case FileNotifyEvent::Modified:
                    ReimportAsset(static_cast<AssetInfo*>(modified.userData));
                    break;
                case FileNotifyEvent::Renamed:
                    static_cast<Asset*>(modified.userData)->GetInfo()->SetName(modified.name);
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

    void AssetManager::WatchAsset(Asset* asset)
    {
        if (asset && hotReloadEnabled)
        {
            fileWatcher.Watch(asset, asset->GetInfo()->GetAbsolutePath());
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
