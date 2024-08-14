#include "AssetHandler.hpp"

#include "Asset.hpp"
#include "AssetManager.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void AssetManagerUpdateUUID(AssetHandler* assetInfo, const UUID& newUUID);
    void AssetManagerUpdatePath(AssetHandler* assetInfo, const StringView& oldPath, const StringView& newPath);

    Asset* AssetHandler::GetInstance() const
    {
        return instance;
    }

    UUID AssetHandler::GetUUID() const
    {
        return uuid;
    }

    void AssetHandler::SetUUID(UUID uuid)
    {
        FY_ASSERT(!this->uuid, "UUID cannot be changed");
        FY_ASSERT(uuid, "UUID cannot be zero");
        if (!this->uuid)
        {
            AssetManagerUpdateUUID(this, uuid);
            this->uuid = uuid;
        }
    }

    TypeHandler* AssetHandler::GetType() const
    {
        return type;
    }

    StringView AssetHandler::GetName() const
    {
        return name;
    }

    StringView AssetHandler::GetPath() const
    {
        return relativePath;
    }

    StringView AssetHandler::GetExtension() const
    {
        return Path::Extension(GetAbsolutePath());
    }

    AssetHandler* AssetHandler::GetParent() const
    {
        return parent;
    }

    Span<AssetHandler*> AssetHandler::GetChildren() const
    {
        return children;
    }

    String AssetHandler::ValidateName(StringView newName)
    {
        u32    count{};
        String finalName = newName;
        bool   nameFound;
        do
        {
            nameFound = true;
            for (AssetHandler* child : parent->GetChildren())
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

    //TODO this needs to be improved
    StringView AssetHandler::GetDisplayName()
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

    void AssetHandler::UpdatePath()
    {
        if (parent != nullptr && !name.Empty())
        {
            name = ValidateName(name);

            String newPath = String().Append(parent->GetPath()).Append("/").Append(name).Append(GetExtension());
            if (relativePath != newPath)
            {
                AssetManagerUpdatePath(this, relativePath, newPath);
            }
            relativePath = newPath;
        }
        else if (!relativePath.Empty())
        {
            AssetManagerUpdatePath(this, relativePath, relativePath);
        }

        if (instance)
        {
            instance->OnPathUpdated();
        }
    }

    ArchiveObject AssetHandler::Serialize(ArchiveWriter& writer) const
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
            for (AssetHandler* child : children)
            {
                writer.AddValue(arr, child->Serialize(writer));
            }
            writer.WriteValue(object, "children", arr);
        }
        return object;
    }

    void AssetHandler::Deserialize(ArchiveReader& reader, ArchiveObject object)
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
                    // AssetHandler* child = AssetManager::CreateAssetInfo();
                    // child->name = reader.ReadString(item, "name");
                    // child->parent = this;
                    // //child->imported = imported;
                    // children.EmplaceBack(child);
                    // child->Deserialize(reader, item);
                }
            }
        }
    }

    void AssetHandler::RemoveChild(AssetHandler* child)
    {
        if (AssetHandler** it = FindFirst(children.begin(), children.end(), child))
        {
            children.Erase(it);
        }
    }

    void AssetHandler::RemoveFromParent()
    {
        if (parent != nullptr)
        {
            parent->RemoveChild(this);
        }
        parent = nullptr;
    }

    bool AssetHandler::IsChildOf(AssetHandler* parent) const
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

    void AssetHandler::AddChild(AssetHandler* child)
    {
        child->RemoveFromParent();
        child->parent = this;
        child->UpdatePath();
        children.EmplaceBack(child);
    }

    AssetHandler* AssetHandler::FindChildByAbsolutePath(StringView absolutePath) const
    {
        for (AssetHandler* child : children)
        {
            if (child->GetAbsolutePath() == absolutePath)
            {
                return child;
            }
        }
        return nullptr;
    }

    void DirectoryAssetHandler::SetName(StringView desiredNewName) {}

    StringView DirectoryAssetHandler::GetAbsolutePath() const
    {
        return absolutePath;
    }

    bool DirectoryAssetHandler::IsModified() const
    {
        return false;
    }

    void DirectoryAssetHandler::SetModified() {}
    void DirectoryAssetHandler::AddRelatedFile(StringView fileAbsolutePath) {}
    void DirectoryAssetHandler::Save() {}
    void DirectoryAssetHandler::Delete() {}

    Asset* DirectoryAssetHandler::LoadInstance()
    {
        return nullptr;
    }

    void DirectoryAssetHandler::UnloadInstance() {}
    void JsonAssetHandler::SetName(StringView desiredNewName) {}

    StringView JsonAssetHandler::GetAbsolutePath() const
    {
        return {};
    }

    bool JsonAssetHandler::IsModified() const
    {
        return false;
    }

    void JsonAssetHandler::SetModified() {}
    void JsonAssetHandler::AddRelatedFile(StringView fileAbsolutePath) {}
    void JsonAssetHandler::Save() {}
    void JsonAssetHandler::Delete() {}

    Asset* JsonAssetHandler::LoadInstance()
    {
        return nullptr;
    }

    void JsonAssetHandler::UnloadInstance() {}

    void ImportedAssetHandler::SetName(StringView desiredNewName) {}

    StringView ImportedAssetHandler::GetAbsolutePath() const
    {
        return {};
    }

    bool ImportedAssetHandler::IsModified() const
    {
        return false;
    }

    void ImportedAssetHandler::SetModified() {}
    void ImportedAssetHandler::AddRelatedFile(StringView fileAbsolutePath) {}
    void ImportedAssetHandler::Save() {}
    void ImportedAssetHandler::Delete() {}

    Asset* ImportedAssetHandler::LoadInstance()
    {
        return nullptr;
    }

    void ImportedAssetHandler::UnloadInstance() {}
}
