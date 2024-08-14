#include "Asset.hpp"

#include "AssetSerialization.hpp"
#include "AssetTypes.hpp"
#include "Fyrion/Core/StringUtils.hpp"
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

    void AssetInfo::SetUUID(UUID uuid)
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
        return type;
    }

    StringView AssetInfo::GetName() const
    {
        return name;
    }

    void AssetInfo::SetName(StringView desiredNewName)
    {
        name = desiredNewName;
    }

    StringView AssetInfo::GetPath() const
    {
        return relativePath;
    }

    StringView AssetInfo::GetAbsolutePath() const
    {
        return absolutePath;
    }

    StringView AssetInfo::GetExtension() const
    {
        return Path::Extension(GetAbsolutePath());
    }

    StringView AssetInfo::GetDisplayName()
    {
        FY_ASSERT(type, "type cannot be null to generate display name");
        if (displayName.Empty() && type)
        {
            displayName = type->GetSimpleName();
            if (usize pos = std::string_view{displayName.CStr(), displayName.Size()}.find("Asset"); pos != nPos)
            {
                displayName.Erase(displayName.begin() + pos, displayName.begin() + pos + 5);
            }
            displayName = FormatName(displayName);
        }
        return displayName;
    }

    AssetInfo* AssetInfo::GetParent() const
    {
        return parent;
    }

    bool AssetInfo::IsModified() const
    {
        return false;
    }

    void AssetInfo::SetModified()
    {
        //do nothing
    }

    void AssetInfo::UpdatePath()
    {
        if (parent != nullptr && !name.Empty())
        {
            name = ValidateName(name);

            String newPath = String().Append(parent->GetPath()).Append("/").Append(name).Append(GetExtension());
            if (relativePath != newPath)
            {
                AssetDatabaseUpdatePath(this, relativePath, newPath);
            }
            relativePath = newPath;
        }
        else if (!relativePath.Empty())
        {
            AssetDatabaseUpdatePath(this, relativePath, relativePath);
        }

        if (instance)
        {
            instance->OnPathUpdated();
        }
    }

    void AssetInfo::AddRelatedFile(StringView fileAbsolutePath)
    {
        //relatedFiles.EmplaceBack(fileAbsolutePath);
    }

    void AssetInfo::Save()
    {

    }

    void AssetInfo::Delete()
    {
      }

    Asset* AssetInfo::LoadInstance()
    {
        if (instance == nullptr && type != nullptr)
        {
            instance = type->Cast<Asset>(type->NewInstance());
            instance->info = this;
        }
        return instance;
    }

    void AssetInfo::UnloadInstance()
    {
        if (instance != nullptr && type != nullptr)
        {
            type->Destroy(instance);
            instance = nullptr;
        }
    }

    Span<AssetInfo*> AssetInfo::GetChildren() const
    {
        return children;
    }

    String AssetInfo::ValidateName(StringView newName)
    {
        u32    count{};
        String finalName = newName;
        bool   nameFound;
        do
        {
            nameFound = true;
            for (AssetInfo* child : parent->GetChildren())
            {
                if (child == this) continue;

                if (finalName == child->name)
                {
                    finalName = newName;
                    finalName += " (";
                    finalName.Append(++count);
                    finalName += ")";
                    nameFound = false;
                    break;
                }
            }
        }
        while (!nameFound);

        return finalName;
    }

    void AssetInfoJson::SetName(StringView desiredNewName)
    {
        if (this->GetName() != desiredNewName)
        {
            String newName = ValidateName(desiredNewName);

            if (!assetPath.Empty())
            {
                auto rename = [this, newName](StringView extension)
                {
                    String oldPath = Path::Join(Path::Parent(assetPath), this->GetName(), extension);
                    String newPath = Path::Join(Path::Parent(assetPath), newName, extension);

                    if (FileSystem::GetFileStatus(oldPath).exists)
                    {
                        FileSystem::Rename(oldPath, newPath);
                    }
                    return newPath;
                };

                String newAbsolutePath = rename(Path::Extension(assetPath));
                rename(FY_IMPORT_EXTENSION);
                rename(FY_INFO_EXTENSION);

                if (imported)
                {
                    String oldPath = Path::Join(Path::Parent(assetPath), this->GetName(), FY_ASSET_EXTENSION);
                    String newPath = Path::Join(Path::Parent(assetPath), newName, FY_ASSET_EXTENSION);
                    if (FileSystem::GetFileStatus(oldPath).exists)
                    {
                        FileSystem::Rename(oldPath, newPath);
                    }
                }

                this->assetPath = newAbsolutePath;
                AssetManager::WatchAsset(this);
            }

            AssetInfo::SetName(newName);
            UpdatePath();
        }
    }

    void AssetInfoJson::Save()
    {
        //TODO
        // String infoPath = Path::Join(Path::Parent(assetInfo->absolutePath), Path::Name(assetInfo->absolutePath), FY_IMPORT_EXTENSION);
        // JsonAssetWriter writer;
        // FileSystem::SaveFileAsString(infoPath, JsonAssetWriter::Stringify(assetInfo->Serialize(writer)));
        // assetInfo->persistedVersion = assetInfo->currentVersion;
    }

    bool AssetInfoJson::IsModified() const
    {
        if (imported) return false;
        return currentVersion != persistedVersion;
    }

    void AssetInfoJson::SetModified()
    {
        currentVersion += 1;
        if (GetInstance())
        {
            GetInstance()->OnModified();
        }
    }

    void AssetInfoJson::AddRelatedFile(StringView fileAbsolutePath)
    {
        relatedFiles.EmplaceBack(fileAbsolutePath);
    }

    void AssetInfoJson::Delete()
    {
        // if (!active) return;
        //
        // active = false;
        //
        // if (instance)
        // {
        //     instance->OnDestroyed();
        //     //TODO - dependencies are not tracked, so destroying any instance could cause an error.
        //     //AssetManager::UnloadAsset(this);
        // }
        //
        // if (FileSystem::GetFileStatus(Path::Join(AssetManager::GetCacheDirectory(), ToString(uuid))).exists)
        // {
        //     FileSystem::Remove(AssetManager::GetCacheDirectory());
        // }
        //
        // auto deleteFile = [this](StringView extension)
        // {
        //     String path = Path::Join(Path::Parent(GetAbsolutePath()), GetName(), extension);
        //     if (FileSystem::GetFileStatus(path).exists)
        //     {
        //         FileSystem::Remove(path);
        //     }
        // };
        //
        // deleteFile(FY_INFO_EXTENSION);
        // deleteFile(FY_IMPORT_EXTENSION);
        //
        // if (!absolutePath.Empty())
        // {
        //     FileSystem::Remove(absolutePath);
        // }
        //
        // for (const String& relatedFile : relatedFiles)
        // {
        //     FileSystem::Remove(relatedFile);
        // }
        //
        // if (parent)
        // {
        //    parent->RemoveChild(this);
        // }
        //
        // AssetDatabaseCleanRefs(this);
    }

    Asset* AssetInfoJson::LoadInstance()
    {
        if (GetInstance() == nullptr)
        {
            Asset* instance = AssetInfo::LoadInstance();

            if (String content = FileSystem::ReadFileAsString(assetPath); !content.Empty())
            {
                JsonAssetReader reader(content);
                instance->Deserialize(reader, reader.ReadObject());
            }
        }
        return GetInstance();
    }

    AssetInfo* Asset::GetInfo() const
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

    void AssetInfo::RemoveChild(AssetInfo* child)
    {
        if (AssetInfo** it = FindFirst(children.begin(), children.end(), child))
        {
            children.Erase(it);
        }
    }

    void AssetInfo::RemoveFromParent()
    {
        if (parent != nullptr)
        {
            parent->RemoveChild(this);
        }
        parent = nullptr;
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

    void AssetInfo::AddChild(AssetInfo* child)
    {
        child->RemoveFromParent();
        child->parent = this;
        child->UpdatePath();
        children.EmplaceBack(child);
    }

    AssetInfo* AssetInfo::FindChildByAbsolutePath(StringView absolutePath) const
    {
        for (AssetInfo* child : children)
        {
            if (child->GetAbsolutePath() == absolutePath)
            {
                return child;
            }
        }
        return nullptr;
    }

    ArchiveObject AssetInfo::Serialize(ArchiveWriter& writer) const
    {
        ArchiveObject object = writer.CreateObject();
        writer.WriteString(object, "uuid", ToString(GetUUID()));
        writer.WriteString(object, "type", GetType()->GetName());

        if (lastModifiedTime != 0)
        {
            writer.WriteUInt(object, "lastModifiedTime", lastModifiedTime);
        }

        if (!children.Empty())
        {
            ArchiveObject arr = writer.CreateArray();
            for(AssetInfo* child: children)
            {
                writer.AddValue(arr, child->Serialize(writer));
            }
            writer.WriteValue(object, "children", arr);
        }
        return object;
    }

    void AssetInfo::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        lastModifiedTime = reader.ReadUInt(object, "lastModifiedTime");
        type = Registry::FindTypeByName(reader.ReadString(object, "type"));
        SetUUID(UUID::FromString(reader.ReadString(object, "uuid")));

        if (ArchiveObject arr = reader.ReadObject(object, "children"))
        {
            auto size = reader.ArrSize(arr);

            ArchiveObject item{};
            for (usize i = 0; i < size; ++i)
            {
                item = reader.Next(arr, item);
                if (item)
                {
                    AssetInfo* child = AssetManager::CreateAssetInfo();
                    child->name = reader.ReadString(item, "name");
                    child->parent = this;
                    //child->imported = imported;
                    children.EmplaceBack(child);
                    child->Deserialize(reader, item);
                }
            }
        }
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {

    }
}
