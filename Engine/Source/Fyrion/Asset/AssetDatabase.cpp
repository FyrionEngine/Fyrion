#include "AssetDatabase.hpp"


namespace Fyrion
{
    HashMap<UUID, Asset*>   assetsById;
    HashMap<String, Asset*> assetsByPath;

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

        for (FieldHandler* field : typeHandler->GetFields())
        {
            TypeHandler* typeHandler = Registry::FindTypeById(field->GetFieldInfo().typeInfo.typeId);
            FY_ASSERT(typeHandler, "Field type not registerd");
            AssetField* assetField = typeHandler->Cast<AssetField>(field->GetFieldPointer(asset));
            if (assetField != nullptr)
            {
                assetField->asset = asset;
                assetField->field = field;
            }
        }

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
        return asset;
    }

    void AssetDatabase::Destroy(Asset* asset)
    {
        asset->assetType->Destroy(asset);
    }

    AssetDirectory* AssetDatabase::LoadFromDirectory(const StringView& name, const StringView& directory)
    {
        return nullptr;
    }


    void AssetDatabaseInit()
    {
    }

    void AssetDatabaseShutdown()
    {
        for (auto& it : assetsById)
        {
            AssetDatabase::Destroy(it.second);
        }
        assetsById.Clear();
        assetsByPath.Clear();
    }
}
