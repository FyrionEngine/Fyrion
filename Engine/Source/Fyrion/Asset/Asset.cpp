#include "Asset.hpp"
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

                if (!child->IsActive()) continue;

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

    void Asset::SetUUID(const UUID& p_uuid)
    {
        uuid = p_uuid;
    }

    TypeID Asset::GetAssetTypeId() const
    {
        return assetType->GetTypeInfo().typeId;
    }

    void Asset::SetName(StringView p_name)
    {
        name = p_name;
        BuildPath();
        Modify();
    }

    void Asset::SetExtension(StringView p_extension)
    {
        extension = p_extension;
    }

    void Asset::SetActive(bool p_active)
    {
        const bool changed = active != p_active;
        active = p_active;
        if (changed)
        {
            OnActiveChanged();
        }
        if (active)
        {
            BuildPath();
        }
        Modify();
    }

    bool Asset::IsChildOf(Asset* parent) const
    {
        if (parent == this) return true;

        if (directory != nullptr)
        {
            if (reinterpret_cast<usize>(this->directory) == reinterpret_cast<usize>(parent))
            {
                return true;
            }

            return directory->IsChildOf(parent);
        }
        return false;
    }

    StringView Asset::GetDisplayName() const
    {
        if (assetType != nullptr)
        {
            return assetType->GetSimpleName();
        }
        return "Asset";
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {
        type.Field<&Asset::uuid, &Asset::GetUUID, &Asset::SetUUID>("uuid");
        type.Field<&Asset::hasBlobs>("hasBlobs");
        type.Function<&Asset::GetName>("GetName");
    }

    Blob Asset::CreateBlob()
    {
        return {Random::Xorshift64star()};
    }

    void Asset::SaveBlob(Blob blob, ConstPtr data, usize dataSize)
    {
        if (blob && storageType == StorageType::Directory)
        {
            StringView dataDirectory = AssetDatabase::GetDataDirectory();
            if (FileSystem::GetFileStatus(dataDirectory).isDirectory)
            {
                String assetDataDirectory = Path::Join(dataDirectory, ToString(GetUUID()));
                if (!FileSystem::GetFileStatus(assetDataDirectory).exists)
                {
                    FileSystem::CreateDirectory(assetDataDirectory);
                }
                String blobPath = Path::Join(assetDataDirectory, blob.ToString());
                FileHandler file = FileSystem::OpenFile(blobPath, AccessMode::WriteOnly);
                FileSystem::WriteFile(file, data, dataSize);
                FileSystem::CloseFile(file);

                hasBlobs = true;
            }
        }
    }

    usize Asset::GetBlobSize(Blob blob) const
    {
        return 0;
    }

    void Asset::LoadBlob(Blob blob, VoidPtr, usize dataSize) const {}
}
