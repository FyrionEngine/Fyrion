#include "AssetDatabase.hpp"

#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{
    namespace
    {
        bool RegisterEvents();

        HashMap<UUID, Asset*>         assetsById;
        HashMap<String, Asset*>       assetsByPath;
        Array<Pair<TypeID, AssetIO*>> assetIOs;
        HashMap<String, AssetIO*>     importers;
        bool                          isShutdown = false;
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
        return Create(typeId, UUID::RandomUUID());
    }

    Asset* AssetDatabase::Create(TypeID typeId, UUID uuid)
    {
        TypeHandler* typeHandler = Registry::FindTypeById(typeId);
        FY_ASSERT(typeHandler, "type not found");

        Asset* asset = typeHandler->Cast<Asset>(typeHandler->NewInstance());
        FY_ASSERT(asset, "type cannot be casted to Asset");

        asset->uniqueId = uuid;
        asset->assetType = typeHandler;
        asset->version = 1;

        assetsById.Insert(asset->uniqueId, asset);

        return asset;
    }

    Asset* AssetDatabase::CreateFromPrototype(TypeID typeId, Asset* prototype)
    {
        return CreateFromPrototype(typeId, prototype, UUID::RandomUUID());
    }

    Asset* AssetDatabase::CreateFromPrototype(TypeID typeId, Asset* prototype, UUID uuid)
    {
        Asset* asset = Create(typeId, uuid);
        asset->prototype = prototype;

        for (FieldHandler* field : asset->GetAssetType()->GetFields())
        {
            auto typeInfo = field->GetFieldInfo().typeInfo;
            if (typeInfo.apiId == GetTypeID<SubobjectApi>())
            {
                SubobjectApi subobjectApi{};
                typeInfo.extractApi(&subobjectApi);
                subobjectApi.SetPrototype(field->GetFieldPointer(asset), field->GetFieldPointer(prototype));
            }
            else if (typeInfo.apiId == GetTypeID<ValueApi>())
            {
                ValueApi valueApi{};
                typeInfo.extractApi(&valueApi);
                valueApi.SetPrototype(field->GetFieldPointer(asset), field->GetFieldPointer(prototype));
            }
        }

        return asset;
    }

    void AssetDatabase::Destroy(Asset* asset, bool destroySubobjects)
    {
        if (destroySubobjects)
        {

            if (asset->subobjectOf)
            {
                SubobjectApi api = asset->subobjectOf->GetApi();
                api.Remove(asset->subobjectOf, asset);
            }

            for (FieldHandler* field : asset->GetAssetType()->GetFields())
            {
                auto typeInfo = field->GetFieldInfo().typeInfo;
                if (typeInfo.apiId == GetTypeID<SubobjectApi>())
                {
                    VoidPtr ptr = field->GetFieldPointer(asset);

                    SubobjectApi subobjectApi{};
                    typeInfo.extractApi(&subobjectApi);

                    TypeID typeId = subobjectApi.GetTypeId();

                    if (TypeHandler* typeHandler = Registry::FindTypeById(typeId))
                    {
                        usize count = subobjectApi.GetOwnedObjectsCount(ptr);
                        Array<VoidPtr> subObjects(count);
                        subobjectApi.GetOwnedObjects(ptr, subObjects);

                        for (VoidPtr subobject : subObjects)
                        {
                            if (Asset* asset = typeHandler->Cast<Asset>(subobject))
                            {
                                Destroy(asset);
                            }
                            else
                            {
                                typeHandler->Destroy(subobject);
                            }

                        }
                    }
                }
            }
        }
        if (!isShutdown)
        {
            assetsById.Erase(asset->GetUniqueId());
            assetsByPath.Erase(asset->GetPath());
        }

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
        assetDirectory->SetPath(String(name) + ":/");

        for (const auto& entry : DirectoryEntries{directory})
        {
            LoadAssetFile(assetDirectory, entry);
        }

        return assetDirectory;
    }

    void AssetDatabase::LoadAssetFile(AssetDirectory* parentDirectory, const StringView& filePath)
    {
        String extension = Path::Extension(filePath);
        if (extension == FY_DATA_EXTENSION) return;

        if (FileSystem::GetFileStatus(filePath).isDirectory)
        {
            AssetDirectory* assetDirectory = Create<AssetDirectory>();
            assetDirectory->SetDirectory(parentDirectory);
            assetDirectory->SetName(Path::Name(filePath));
            parentDirectory->children.Add(assetDirectory);

            for (const auto& entry : DirectoryEntries{filePath})
            {
                LoadAssetFile(assetDirectory, entry);
            }
        }
        else if (extension == FY_ASSET_EXTENSION)
        {
            //TODO
        }
        else if (auto importer = importers.Find(extension))
        {
            if (Asset* asset = importer->second->ImportAsset(filePath, nullptr))
            {
                if (asset->GetName().Empty())
                {
                    asset->SetName(Path::Name(filePath) + Path::Extension(filePath));
                }
                asset->SetDirectory(parentDirectory);
                parentDirectory->children.Add(asset);
            }
        }
    }

    AssetDirectory* AssetDatabase::LoadFromFile(const StringView& name, const StringView& file)
    {
        FY_ASSERT(false, "not implemented");
        return nullptr;
    }

    void AssetDatabaseInit()
    {
    }

    void AssetDatabaseShutdown()
    {
        isShutdown = true;
        for (auto& it : assetsById)
        {
            AssetDatabase::Destroy(it.second, false);
        }
        isShutdown = false;

        for (const auto& assetIo : assetIOs)
        {
            if (TypeHandler* typeHandler = Registry::FindTypeById(assetIo.first))
            {
                typeHandler->Destroy(assetIo.second);
            }
        }

        assetIOs.Clear();
        assetIOs.ShrinkToFit();

        assetsById.Clear();
        assetsByPath.Clear();
        importers.Clear();
    }
}
