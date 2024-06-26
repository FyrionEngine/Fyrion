#include "Asset.hpp"
#include "AssetTypes.hpp"

namespace Fyrion
{
    void AssetDatabaseUpdatePath(Asset* asset, const StringView& oldPath, const StringView& newPath);

    void Asset::BuildPath()
    {
        if (directory != nullptr && !name.Empty())
        {
            ValidateName();
            String newPath = String().Append(directory->GetPath()).Append("/").Append(name);
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

    void Asset::SetName(StringView p_name)
    {
        name = p_name;
        BuildPath();
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

    void Asset::RegisterType(NativeTypeHandler<Asset>& type) {}
}
