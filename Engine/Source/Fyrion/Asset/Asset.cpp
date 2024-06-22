#include "Asset.hpp"
#include "AssetTypes.hpp"

namespace Fyrion
{
    void AssetDatabaseUpdatePath(Asset* asset, const StringView& oldPath, const StringView& newPath);

    void Asset::BuildPath()
    {
        if (directory != nullptr && !name.Empty())
        {
            SetPath(String().Append(directory->GetPath()).Append("/").Append(name));
        }
    }

    void Asset::SetPath(StringView p_path)
    {
        AssetDatabaseUpdatePath(this, path, p_path);
        path = p_path;
    }

    void Asset::SetName(StringView p_name)
    {
        name = p_name;
        BuildPath();
    }

    void Asset::SetDirectory(AssetDirectory* p_directory)
    {
        directory = p_directory;
        BuildPath();
    }

    bool Asset::IsParentOf(Asset* asset) const
    {
        if (asset == this) return false;

        if (directory != nullptr)
        {
            if (reinterpret_cast<usize>(asset->GetDirectory()) == reinterpret_cast<usize>(asset))
            {
                return true;
            }

            return directory->IsParentOf(asset);
        }
        return false;
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {
    }
}
