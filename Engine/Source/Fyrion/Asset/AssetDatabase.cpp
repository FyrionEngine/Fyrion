#include "AssetDatabase.hpp"

#include "Fyrion/IO/FileSystem.hpp"


namespace Fyrion
{
    HashMap<UUID, Asset*>   assetsById;
    HashMap<String, Asset*> assetsByPath;
    bool isShutdown = false;

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
            for (FieldHandler* field : asset->GetAssetType()->GetFields())
            {
                auto typeInfo = field->GetFieldInfo().typeInfo;
                if (typeInfo.apiId == GetTypeID<SubobjectApi>())
                {
                    VoidPtr ptr = field->GetFieldPointer(asset);

                    SubobjectApi subobjectApi{};
                    typeInfo.extractApi(&subobjectApi);

                    usize count = subobjectApi.GetOwnedObjectsCount(ptr);
                    Array<Asset*> subObjects(count);
                    subobjectApi.GetOwnedObjects(ptr, subObjects);

                    for(Asset* subobject: subObjects)
                    {
                        Destroy(subobject);
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


        return assetDirectory;
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

        assetsById.Clear();
        assetsByPath.Clear();
    }
}
