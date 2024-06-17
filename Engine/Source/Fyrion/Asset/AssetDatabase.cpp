#include "AssetDatabase.hpp"


namespace Fyrion
{
    HashMap<UUID, Asset*>   assetsById;
    HashMap<String, Asset*> assetsByPath;

    Asset* AssetDatabase::Instantiate(TypeID typeId)
    {
        Registry::FindTypeById(typeId);


       return nullptr;
    }

    void AssetDatabase::Destroy(Asset* asset)
    {
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
        for(auto& it : assetsById)
        {
            AssetDatabase::Destroy(it.second);
        }
        assetsById.Clear();
        assetsByPath.Clear();
    }
}
