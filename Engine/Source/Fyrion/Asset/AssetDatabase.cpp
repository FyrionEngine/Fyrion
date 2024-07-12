#include "AssetDatabase.hpp"

#include "AssetSerialization.hpp"
#include "AssetTypes.hpp"
#include "StreamObject.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{
    namespace
    {
        bool RegisterEvents();

        String                        dataDirectory;
        Array<Asset*>                 assets;
        HashMap<UUID, Asset*>         assetsById;
        HashMap<String, Asset*>       assetsByPath;
        Array<Pair<TypeID, AssetIO*>> assetIOs;
        HashMap<String, AssetIO*>     importers;
        Logger&                       logger = Logger::GetLogger("Fyrion::AssetDatabase", LogLevel::Debug);
        bool                          registerEvents = RegisterEvents();

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

    Asset* AssetDatabase::Create(TypeID typeId)
    {
        return Create(typeId, {});
    }

    Asset* AssetDatabase::Create(TypeID typeId, UUID uuid)
    {
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

        assetDirectory->loadedVersion =  assetDirectory->currentVersion;

        return assetDirectory;
    }

    void AssetDatabase::LoadAssetFile(AssetDirectory* parentDirectory, const StringView& filePath)
    {
        String extension = Path::Extension(filePath);
        if (extension == FY_DATA_EXTENSION) return;

        if (FileSystem::GetFileStatus(filePath).isDirectory)
        {
            AssetDirectory* assetDirectory = Create<AssetDirectory>();
            assetDirectory->absolutePath = filePath;
            assetDirectory->loadedVersion =  assetDirectory->currentVersion;
            assetDirectory->name = Path::Name(filePath);
            parentDirectory->AddChild(assetDirectory);

            for (const auto& entry : DirectoryEntries{filePath})
            {
                LoadAssetFile(assetDirectory, entry);
            }
        }
        else if (extension == FY_ASSET_EXTENSION)
        {
            FileHandler handler = FileSystem::OpenFile(filePath, AccessMode::ReadOnly);
            u64 size = FileSystem::GetFileSize(handler);
            String buffer(size);
            FileSystem::ReadFile(handler, buffer.begin(), buffer.Size());
            FileSystem::CloseFile(handler);

            JsonAssetReader reader(buffer);
            ArchiveObject root = reader.ReadObject();
            if (TypeHandler* typeHandler = Registry::FindTypeByName(reader.ReadString(root, "_type")))
            {
                Asset* asset = Create(typeHandler->GetTypeInfo().typeId);
                Serialization::Deserialize(typeHandler, reader, root, asset);

                AssetDatabaseUpdateUUID(asset, asset->GetUUID());

                asset->name = Path::Name(filePath);
                asset->extension = Path::Extension(filePath);
                asset->absolutePath = filePath;
                asset->loadedVersion =  asset->currentVersion;
                parentDirectory->AddChild(asset);
            }
        }
        else if (auto importer = importers.Find(extension))
        {
            if (Asset* asset = importer->second->CreateAsset())
            {
                asset->name = Path::Name(filePath);
                asset->absolutePath = filePath;
                asset->extension = Path::Extension(filePath);
                parentDirectory->AddChild(asset);

                AssetDatabaseUpdateUUID(asset, asset->GetUUID());

                importer->second->ImportAsset(filePath, asset);
            }
        }
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
                String assetPath = Path::Join(directoryPath, asset->GetName(), asset->GetInfoExtension());
                if (assetPath != asset->GetAbsolutePath() && oldPathExists)
                {
                    FileSystem::Remove(asset->GetAbsolutePath());
                    logger.Debug("Asset {} moved from {} to {} ", asset->GetPath(), asset->GetAbsolutePath(), assetPath);
                }

                JsonAssetWriter writer;

                ArchiveObject object = Serialization::Serialize(asset->GetAssetType(), writer, asset);
                writer.WriteString(object, "_type", asset->GetAssetType()->GetName());
                String str = JsonAssetWriter::Stringify(object);

                FileHandler handler = FileSystem::OpenFile(assetPath, AccessMode::WriteOnly);
                FileSystem::WriteFile(handler, str.begin(), str.Size());
                FileSystem::CloseFile(handler);

                asset->absolutePath = assetPath;
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



    AssetDirectory* AssetDatabase::LoadFromFile(const StringView& name, const StringView& file)
    {
        FY_ASSERT(false, "not implemented");
        return nullptr;
    }

    Asset* AssetDatabase::ImportAsset(AssetDirectory* directory, const StringView& path)
    {
        return nullptr;
    }

    void AssetDatabaseInit() {}

    void AssetDatabaseShutdown()
    {
        for (Asset* asset : assets)
        {
            asset->GetAssetType()->Destroy(asset);
        }

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

        assets.Clear();
        assets.ShrinkToFit();

        assetsById.Clear();
        assetsByPath.Clear();
        importers.Clear();
    }

    void StreamObject::Init(UUID p_uniqueId)
    {
        uniqueId = p_uniqueId;

        //.pak
        //.data
    }

    void StreamObject::Save(ConstPtr data, usize size)
    {
        if (!uniqueId)
        {
            uniqueId = UUID::RandomUUID();
        }

        if (!dataDirectory.Empty())
        {
            // String fullPath = Path::Join(dataDirectory, "Objects", ToString(uniqueId));
            // FileHandler fileHandler = FileSystem::OpenFile(fullPath, AccessMode::WriteOnly);
            // FileSystem::WriteFile(fileHandler, data, size);
            // FileSystem::CloseFile(fileHandler);
        }
    }

    usize StreamObject::Size() const
    {
        return 0;
    }

    void StreamObject::Load(VoidPtr data) const
    {

    }
}
