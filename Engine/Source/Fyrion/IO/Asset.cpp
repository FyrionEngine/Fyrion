#include "Asset.hpp"

#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    namespace
    {
        struct AssetCache
        {
            UUID         uuid;
            AssetLoader* loader;
            Asset*       instance;
        };

        HashMap<UUID, AssetCache> assetCache = {};
        HashMap<String, UUID> assetsByPath = {};
    }

    UUID Asset::GetUUID() const
    {
        return uuid;
    }

    TypeHandler* Asset::GetTypeHandler() const
    {
        return typeHandler;
    }

    Array<u8> Asset::LoadStream(usize offset, usize size) const
    {
        return loader->LoadStream(offset, size);
    }

    void Asset::SetTypeHandler(TypeHandler* typeHandler)
    {
        this->typeHandler = typeHandler;
    }

    //---------------assets------------

    void AssetsShutdown()
    {
        for(auto it : assetCache)
        {
            if (it.second.instance)
            {
                it.second.instance->GetTypeHandler()->Destroy(it.second.instance);
                it.second.instance = nullptr;
            }
        }
        assetCache.Clear();
        assetsByPath.Clear();
    }

    void Assets::Create(UUID uuid, AssetLoader* loader)
    {
        assetCache.Emplace(uuid, AssetCache{
                               .uuid = uuid,
                               .loader = loader,
                               .instance = nullptr
                           });
    }

    Asset* Assets::Load(UUID uuid)
    {
        if (auto it = assetCache.Find(uuid))
        {
            if(!it->second.instance && it->second.loader)
            {
                it->second.instance = it->second.loader->LoadAsset();
                FY_ASSERT(it->second.instance, "instance not created");
                FY_ASSERT(it->second.instance->typeHandler, "type handler must be provided");
                it->second.instance->loader = it->second.loader;
                it->second.instance->uuid = it->second.uuid;
            }
            return it->second.instance;
        }
         return nullptr;
    }

    Asset* Assets::Reload(UUID uuid)
    {
        if (auto it = assetCache.Find(uuid))
        {
            if(it->second.instance)
            {
                it->second.instance->GetTypeHandler()->Destroy(it->second.instance);
            }

            it->second.instance = it->second.loader->LoadAsset();
            FY_ASSERT(it->second.instance, "instance not created");
            FY_ASSERT(it->second.instance->typeHandler, "type handler must be provided");
            it->second.instance->loader = it->second.loader;
            it->second.instance->uuid = it->second.uuid;
            return it->second.instance;
        }
        return nullptr;
    }

    Asset* Assets::LoadByPath(StringView path)
    {
        if (auto it = assetsByPath.Find(path))
        {
            return Load(it->second);
        }
        return nullptr;
    }

    void Assets::SetPath(UUID uuid, StringView path)
    {
        assetsByPath.Insert(path, uuid);
    }
}
