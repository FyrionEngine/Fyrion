#include "AssetManager.hpp"

#include "Asset.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Core/UUID.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{
    namespace
    {
        HashMap<UUID, SharedPtr<Asset>> assetsByUUID;
    }
}
