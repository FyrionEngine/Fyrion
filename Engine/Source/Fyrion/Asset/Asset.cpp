#include "Asset.hpp"

#include "AssetSerialization.hpp"
#include "AssetTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void AssetDatabaseUpdatePath(AssetInfo* assetInfo, const StringView& oldPath, const StringView& newPath);
    void AssetDatabaseUpdateUUID(AssetInfo* assetInfo, const UUID& newUUID);
    void AssetDatabaseCleanRefs(AssetInfo* assetInfo);

    Asset* AssetInfo::GetInstance() const
    {
        return instance;
    }

    UUID AssetInfo::GetUUID() const
    {
        return uuid;
    }

    void AssetInfo::SetUUID()
    {
        FY_ASSERT(!this->uuid, "UUID cannot be changed");
        FY_ASSERT(uuid, "UUID cannot be zero");
        if (!this->uuid)
        {
            AssetDatabaseUpdateUUID(this, uuid);
            this->uuid = uuid;
        }
    }

    TypeHandler* AssetInfo::GetType() const
    {
        return assetType;
    }

    StringView AssetInfo::GetName() const
    {
        return name;
    }

    void AssetInfo::SetName(StringView newName)
    {

    }

    StringView AssetInfo::GetPath() const
    {
        return path;
    }

    StringView AssetInfo::GetAbsolutePath() const
    {
        return absolutePath;
    }

    StringView AssetInfo::GetExtension() const
    {
        return Path::Extension(absolutePath);
    }

    StringView AssetInfo::GetDisplayName() const
    {
        return "Asset";
    }

    AssetInfo* AssetInfo::GetParent() const
    {
        return parent;
    }

    bool AssetInfo::IsChildOf(AssetInfo* parent) const
    {
        if (parent == this) return true;

        if (this->parent != nullptr)
        {
            if (this->parent == parent)
            {
                return true;
            }

            return this->parent->IsChildOf(parent);
        }
        return false;
    }

    bool AssetInfo::IsModified() const
    {
        return false;
    }

    void AssetInfo::SetModified()
    {

    }

    void AssetInfo::UpdatePath()
    {
        if (parent != nullptr && !name.Empty())
        {
            ValidateName();

            String newPath = String().Append(parent->GetPath()).Append("/").Append(name).Append(GetExtension());
            if (path != newPath)
            {
                AssetDatabaseUpdatePath(this, path, newPath);
            }
            path = newPath;
        }
    }

    void AssetInfo::AddRelatedFile(StringView fileAbsolutePath)
    {
        relatedFiles.EmplaceBack(fileAbsolutePath);
    }

    void AssetInfo::Destroy()
    {
        if (instance)
        {
          //  instance->OnDestroyed();
        }

        if (FileSystem::GetFileStatus(GetCacheDirectory()).exists)
        {
            FileSystem::Remove(GetCacheDirectory());
        }

        auto deleteFile = [this](StringView extension)
        {
            String path = Path::Join(Path::Parent(GetAbsolutePath()), GetName(), extension);
            if (FileSystem::GetFileStatus(path).exists)
            {
                FileSystem::Remove(path);
            }
        };

        deleteFile(FY_INFO_EXTENSION);
        deleteFile(FY_IMPORT_EXTENSION);

        if (!absolutePath.Empty())
        {
            FileSystem::Remove(absolutePath);
        }

        for (const String& relatedFile : relatedFiles)
        {
            FileSystem::Remove(relatedFile);
        }

        if (parent)
        {
         //   parent->RemoveChild(this);
        }

        AssetDatabaseCleanRefs(this);
    }

    String AssetInfo::GetCacheDirectory() const
    {
        String parentCacheDir = parent != nullptr ? parent->GetCacheDirectory() : "";
        if (!parentCacheDir.Empty())
        {
            return Path::Join(parentCacheDir, ToString(uuid));
        }
        return Path::Join(AssetManager::GetCacheDirectory(), ToString(uuid));
    }

    void AssetInfo::ValidateName()
    {
        DirectoryInfo* parent = dynamic_cast<DirectoryInfo*>(this->parent);

        u32    count{};
        String finalName = name;
        bool   nameFound;
        do
        {
            nameFound = true;
            for (AssetInfo* child : parent->GetChildren())
            {
                if (child == this) continue;

                if (finalName == child->name)
                {
                    finalName = name;
                    finalName += " (";
                    finalName.Append(++count);
                    finalName += ")";
                    nameFound = false;
                    break;
                }
            }
        }
        while (!nameFound);

        if (name != finalName)
        {
            name = finalName;
        }
    }

    AssetInfo* Asset::GetInfo() const
    {
        return info;
    }

#if 0
    void Asset::SetName(StringView newName)
    {

        if (this->name != newName)
        {
            if (!absolutePath.Empty())
            {
                auto rename = [this, newName](StringView extension)
                {
                    String oldPath = Path::Join(Path::Parent(absolutePath), this->name, extension);
                    String newPath = Path::Join(Path::Parent(absolutePath), newName, extension);

                    if (FileSystem::GetFileStatus(oldPath).exists)
                    {
                        FileSystem::Rename(oldPath, newPath);
                    }
                    return newPath;
                };

                String newAbsolutePath = rename(Path::Extension(absolutePath));
                rename(FY_IMPORT_EXTENSION);
                rename(FY_INFO_EXTENSION);

                this->absolutePath = newAbsolutePath;
                AssetManager::WatchAsset(this);
            }

            this->name = newName;
            BuildPath();
            SetModified();
        }
    }
#endif


    // void Asset::SetModified()
    // {
    //     currentVersion += 1;
    //     OnModified();
    // }
    //
    // bool Asset::IsModified() const
    // {
    //     if (imported) return false;
    //
    //     return currentVersion != loadedVersion;
    // }


    void Asset::SaveCache(CacheRef& cache, ConstPtr data, usize dataSize)
    {
        if (!cache)
        {
            cache.id = Random::Xorshift64star();
        }

        String cacheDirectory = info->GetCacheDirectory();
        if (!cacheDirectory.Empty())
        {
            if (!FileSystem::GetFileStatus(cacheDirectory).exists)
            {
                FileSystem::CreateDirectory(cacheDirectory);
            }

            String      streamPath = Path::Join(cacheDirectory, cache.ToString());
            FileHandler file = FileSystem::OpenFile(streamPath, AccessMode::WriteOnly);
            FileSystem::WriteFile(file, data, dataSize);
            FileSystem::CloseFile(file);
        }
    }

    usize Asset::GetCacheSize(CacheRef cache) const
    {
        String cacheDirectory = info->GetCacheDirectory();
        if (!cacheDirectory.Empty())
        {
            String streamPath = Path::Join(cacheDirectory, cache.ToString());
            return FileSystem::GetFileStatus(streamPath).fileSize;
        }
        return 0;
    }

    void Asset::LoadCache(CacheRef cache, VoidPtr data, usize dataSize) const
    {
        String cacheDirectory = info->GetCacheDirectory();
        if (!cacheDirectory.Empty())
        {
            String streamPath = Path::Join(cacheDirectory, cache.ToString());
            FileHandler file = FileSystem::OpenFile(streamPath, AccessMode::ReadOnly);
            FileSystem::ReadFile(file, data, dataSize);
            FileSystem::CloseFile(file);
        }
    }

    // void Asset::RemoveChild(Asset* child)
    // {
    //     if (Asset** it = FindFirst(children.begin(), children.end(), child))
    //     {
    //         children.Erase(it);
    //     }
    // }
    //
    // void Asset::RemoveFromParent()
    // {
    //     if (parent != nullptr)
    //     {
    //         parent->RemoveChild(this);
    //     }
    //     parent = nullptr;
    // }

    // void Asset::AddChild(Asset* child)
    // {
    //     child->RemoveFromParent();
    //     child->parent = this;
    //     child->BuildPath();
    //     children.EmplaceBack(child);
    // }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {

    }
}
