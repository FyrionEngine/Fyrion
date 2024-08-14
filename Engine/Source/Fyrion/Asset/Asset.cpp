#include "Asset.hpp"

#include "AssetHandler.hpp"
#include "AssetSerialization.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void AssetDatabaseCleanRefs(AssetHandler* assetInfo);

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::Asset", LogLevel::Debug);
    }


    AssetHandler* Asset::GetInfo() const
    {
        return info;
    }

    void Asset::SetModified()
    {
        info->SetModified();
    }

    ArchiveObject Asset::Serialize(ArchiveWriter& writer) const
    {
        return Serialization::Serialize(info->GetType(), writer, this);
    }

    void Asset::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        Serialization::Deserialize(info->GetType(), reader, object, this);
    }

    void Asset::SaveCache(CacheRef& cache, ConstPtr data, usize dataSize)
    {
        if (!cache)
        {
            cache.id = Random::Xorshift64star();
        }

        String cacheDir = Path::Join(AssetManager::GetCacheDirectory(), ToString(info->GetUUID()));
        if (!cacheDir.Empty())
        {
            if (!FileSystem::GetFileStatus(cacheDir).exists)
            {
                FileSystem::CreateDirectory(cacheDir);
            }
            String      streamPath = Path::Join(cacheDir, cache.ToString());
            FileHandler file = FileSystem::OpenFile(streamPath, AccessMode::WriteOnly);
            FileSystem::WriteFile(file, data, dataSize);
            FileSystem::CloseFile(file);
        }
    }

    usize Asset::GetCacheSize(CacheRef cache) const
    {
        String cacheDir = Path::Join(AssetManager::GetCacheDirectory(), ToString(info->GetUUID()));
        if (!cacheDir.Empty())
        {
            String streamPath = Path::Join(cacheDir, cache.ToString());
            return FileSystem::GetFileStatus(streamPath).fileSize;
        }
        return 0;
    }

    void Asset::LoadCache(CacheRef cache, VoidPtr data, usize dataSize) const
    {
        String cacheDir = Path::Join(AssetManager::GetCacheDirectory(), ToString(info->GetUUID()));
        if (!cacheDir.Empty())
        {
            String streamPath = Path::Join(cacheDir, cache.ToString());
            FileHandler file = FileSystem::OpenFile(streamPath, AccessMode::ReadOnly);
            FileSystem::ReadFile(file, data, dataSize);
            FileSystem::CloseFile(file);
        }
    }

    Asset* Asset::GetParent() const
    {
        if (info->GetParent() != nullptr)
        {
            return info->GetParent()->LoadInstance();
        }
        return nullptr;
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {

    }
}
