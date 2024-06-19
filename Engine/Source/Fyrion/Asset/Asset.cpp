#include "Asset.hpp"

namespace Fyrion
{
    void Asset::SetPath(StringView p_path)
    {
        path = p_path;
    }

    void Asset::SetName(StringView p_name)
    {
        name = p_name;
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {
    }
}
