#include "Asset.hpp"

#include "AssetSerialization.hpp"
#include "AssetTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void AssetDatabaseUpdatePath(Asset* asset, const StringView& oldPath, const StringView& newPath);
    void AssetDatabaseUpdateUUID(Asset* asset, const UUID& newUUID);
    void AssetDatabaseCleanRefs(Asset* asset);

    void Asset::BuildPath()
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

            for (Asset* child : children)
            {
                String newPathChild = String().Append(newPath).Append("#").Append(child->name);
                AssetDatabaseUpdatePath(child, child->path, newPathChild);
            }
        }
    }

    void Asset::ValidateName()
    {
        u32    count{};
        String finalName = name;
        bool   nameFound;
        do
        {
            nameFound = true;
            for (Asset* child : parent->GetChildren())
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

    UUID Asset::GetUUID() const
    {
        return uuid;
    }

    void Asset::SetUUID(const UUID& uuid)
    {
        FY_ASSERT(!this->uuid, "UUID cannot be changed");
        FY_ASSERT(uuid, "UUID cannot be zero");
        if (!this->uuid)
        {
            AssetDatabaseUpdateUUID(this, uuid);
            this->uuid = uuid;
        }
    }

    TypeHandler* Asset::GetType() const
    {
        return assetType;
    }

    TypeID Asset::GetAssetTypeId() const
    {
        return assetType->GetTypeInfo().typeId;
    }

    Asset* Asset::GetPrototype() const
    {
        return prototype;
    }

    StringView Asset::GetName() const
    {
        return name;
    }

    StringView Asset::GetPath() const
    {
        return path;
    }

    Asset* Asset::GetParent() const
    {
        return parent;
    }

    StringView Asset::GetAbsolutePath() const
    {
        return absolutePath;
    }

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
            }

            this->name = newName;
            BuildPath();
            SetModified();
        }

    }

    void Asset::AddRelatedFile(StringView fileAbsolutePath)
    {
        relatedFiles.EmplaceBack(fileAbsolutePath);
    }

    StringView Asset::GetExtension() const
    {
        return Path::Extension(absolutePath);
    }

    bool Asset::IsChildOf(Asset* parent) const
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

    ImportSettings* Asset::GetImportSettings()
    {
        return nullptr;
    }

    void Asset::SetModified()
    {
        currentVersion += 1;
        OnModified();
    }

    bool Asset::IsModified() const
    {
        if (imported) return false;

        return currentVersion != loadedVersion;
    }

    StringView Asset::GetDisplayName() const
    {
        if (assetType != nullptr)
        {
            return assetType->GetSimpleName();
        }
        return "Asset";
    }

    void Asset::SaveCache(CacheRef& cache, ConstPtr data, usize dataSize)
    {
        if (!cache)
        {
            cache.id = Random::Xorshift64star();
        }

        String cacheDirectory = GetCacheDirectory();
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
        String cacheDirectory = GetCacheDirectory();
        if (!cacheDirectory.Empty())
        {
            String streamPath = Path::Join(cacheDirectory, cache.ToString());
            return FileSystem::GetFileStatus(streamPath).fileSize;
        }
        return 0;
    }

    void Asset::LoadCache(CacheRef cache, VoidPtr data, usize dataSize) const
    {
        String cacheDirectory = GetCacheDirectory();
        if (!cacheDirectory.Empty())
        {
            String streamPath = Path::Join(cacheDirectory, cache.ToString());
            FileHandler file = FileSystem::OpenFile(streamPath, AccessMode::ReadOnly);
            FileSystem::ReadFile(file, data, dataSize);
            FileSystem::CloseFile(file);
        }
    }

    Span<Asset*> Asset::GetChildren() const
    {
        return children;
    }

    void Asset::RemoveChild(Asset* child)
    {
        if (Asset** it = FindFirst(children.begin(), children.end(), child))
        {
            children.Erase(it);
        }
    }

    void Asset::RemoveFromParent()
    {
        if (parent != nullptr)
        {
            parent->RemoveChild(this);
        }
        parent = nullptr;
    }

    void Asset::AddChild(Asset* child)
    {
        child->RemoveFromParent();
        child->parent = this;
        child->BuildPath();
        children.EmplaceBack(child);
    }

    Asset* Asset::FindChildByAbsolutePath(StringView absolutePath) const
    {
        for (Asset* child : children)
        {
            if (child->absolutePath == absolutePath)
            {
                return child;
            }
        }
        return nullptr;
    }

    String Asset::GetCacheDirectory() const
    {
        String parentCacheDir = parent != nullptr ? parent->GetCacheDirectory() : "";
        if (!parentCacheDir.Empty())
        {
            return Path::Join(parentCacheDir, ToString(uuid));
        }
        return Path::Join(AssetDatabase::GetCacheDirectory(), ToString(uuid));
    }

    void Asset::Destroy()
    {
        OnDestroyed();

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
            parent->RemoveChild(this);
        }

        AssetDatabaseCleanRefs(this);
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {
        type.Field<&Asset::name>("name");
        type.Function<&Asset::GetName>("GetName");
    }
}
