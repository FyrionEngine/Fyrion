#include "Asset.hpp"

namespace Fyrion
{

    void AssetDatabaseUpdatePath(Asset* asset, const StringView& newPath);

    void Asset::SetPath(StringView p_path)
    {
        path = p_path;
        AssetDatabaseUpdatePath(this, path);
    }

    void Asset::SetName(StringView p_name)
    {
        name = p_name;
        if (directory != nullptr)
        {
            SetPath(String().Append(directory->GetPath()).Append("/").Append(p_name));
        }
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {
    }
}
