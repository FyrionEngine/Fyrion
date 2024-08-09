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

    void Asset::BuildPath()
    {
        if (directory != nullptr && !name.Empty())
        {
            ValidateName();
            String newPath = String().Append(directory->GetPath()).Append("/").Append(name).Append(extension);
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
            for (Asset* child : directory->GetChildren())
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

    Asset::~Asset()
    {
        // if (directory)
        // {
        //     directory->RemoveChild(this);
        // }
    }

    UUID Asset::GetUUID() const
    {
        return uuid;
    }

    void Asset::SetUUID(const UUID& uuid)
    {
        AssetDatabaseUpdateUUID(this, uuid);
        this->uuid = uuid;
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

    void Asset::SetName(StringView name)
    {
        this->name = name;
        BuildPath();
        SetModified();
    }

    void Asset::AddRelatedFile(StringView fileAbsolutePath)
    {
        relatedFiles.EmplaceBack(fileAbsolutePath);
    }

    void Asset::SetExtension(StringView p_extension)
    {
        extension = p_extension;
    }

    StringView Asset::GetExtension() const
    {
        return extension;
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

    void Asset::SetModified()
    {
        currentVersion += 1;
        OnModified();
    }

    bool Asset::IsModified() const
    {
        if (loadedVersion == 0)
        {
            return false;
        }
        if (!GetUUID())
        {
            return false;
        }
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

    void Asset::SaveBlob(Blob& blob, ConstPtr data, usize dataSize)
    {
        Asset* physicalAsset = GetPhysicalAsset();
        FY_ASSERT(physicalAsset->GetPhysicalAsset(), "blob cannot be saved, no physical asset found");

        if (!blob)
        {
            blob.id = Random::Xorshift64star();
        }

        StringView dataDirectory = AssetDatabase::GetCacheDirectory();
        if (FileSystem::GetFileStatus(dataDirectory).isDirectory)
        {
            String assetDataDirectory = Path::Join(dataDirectory, ToString(physicalAsset->GetUUID()));
            if (!FileSystem::GetFileStatus(assetDataDirectory).exists)
            {
                FileSystem::CreateDirectory(assetDataDirectory);
            }
            String      blobPath = Path::Join(assetDataDirectory, blob.ToString());
            FileHandler file = FileSystem::OpenFile(blobPath, AccessMode::WriteOnly);
            FileSystem::WriteFile(file, data, dataSize);
            FileSystem::CloseFile(file);

            physicalAsset->hasBlobs = true;
        }
    }

    usize Asset::GetBlobSize(Blob blob) const
    {
        StringView dataDirectory = AssetDatabase::GetCacheDirectory();
        const Asset* physicalAsset = GetPhysicalAsset();
        if (physicalAsset != nullptr && FileSystem::GetFileStatus(dataDirectory).isDirectory)
        {
            String blobPath = Path::Join(Path::Join(dataDirectory, ToString(physicalAsset->GetUUID())), blob.ToString());
            return FileSystem::GetFileStatus(blobPath).fileSize;
        }
        return 0;
    }

    void Asset::LoadBlob(Blob blob, VoidPtr data, usize dataSize) const
    {
        StringView dataDirectory = AssetDatabase::GetCacheDirectory();
        const Asset* physicalAsset = GetPhysicalAsset();
        if (physicalAsset != nullptr && FileSystem::GetFileStatus(dataDirectory).isDirectory)
        {
            String      blobPath = Path::Join(Path::Join(dataDirectory, ToString(physicalAsset->GetUUID())), blob.ToString());
            FileHandler file = FileSystem::OpenFile(blobPath, AccessMode::ReadOnly);
            FileSystem::ReadFile(file, data, dataSize);
        }
    }

    Span<Asset*> Asset::GetChildrenAssets() const
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

    void Asset::AddChild(Asset* child)
    {
        if (child->parent != nullptr)
        {
            child->parent->RemoveChild(child);
        }

        child->directory = this;
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

    Asset* Asset::GetPhysicalAsset()
    {
        if (directory != nullptr)
        {
            return this;
        }

        if (parent != nullptr)
        {
            return parent->GetPhysicalAsset();
        }

        return {};
    }

    const Asset* Asset::GetPhysicalAsset() const
    {
        if (directory != nullptr)
        {
            return this;
        }

        if (parent != nullptr)
        {
            return parent->GetPhysicalAsset();
        }

        return {};
    }

    void Asset::SetOwner(Asset* p_owner)
    {
        FY_ASSERT(!parent, "asset already have a owner");
        if (p_owner != this)
        {
            parent = p_owner;
            parent->children.EmplaceBack(this);
        }
    }

    bool Asset::LoadData()
    {
        StringView dataExtension = GetDataExtesion();
        if (!dataExtension.Empty())
        {
            if (!absolutePath.Empty())
            {
                String buffer = FileSystem::ReadFileAsString(Path::Join(Path::Parent(absolutePath), Path::Name(absolutePath), dataExtension));
                if (!buffer.Empty())
                {
                    JsonAssetReader reader(buffer);
                    DeserializeData(reader, reader.ReadObject());
                    return true;
                }
            }
        }
        return false;
    }

    void Asset::SaveData()
    {
        StringView dataExtension = GetDataExtesion();
        if (!dataExtension.Empty())
        {
            if (!absolutePath.Empty())
            {
                String dataPath = Path::Join(Path::Parent(absolutePath), Path::Name(absolutePath), dataExtension);
                JsonAssetWriter writer{};
                if (ArchiveObject object = SerializeData(writer))
                {
                    FileSystem::SaveFileAsString(dataPath, JsonAssetWriter::Stringify(object));
                }
            }
        }
    }

    StringView Asset::GetDataExtesion()
    {
        return {};
    }

    void Asset::DeserializeData(ArchiveReader& reader, ArchiveObject object)
    {
    }

    ArchiveObject Asset::SerializeData(ArchiveWriter& writer) const
    {
        return {};
    }

    void Asset::Destroy()
    {
        OnDestroyed();

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

        //TODO - remove references from AssetDatabase.
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {
        type.Field<&Asset::name>("name");
        type.Field<&Asset::hasBlobs>("hasBlobs");
        type.Function<&Asset::GetName>("GetName");
    }
}
