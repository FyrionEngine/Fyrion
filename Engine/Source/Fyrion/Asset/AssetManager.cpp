#include "AssetManager.hpp"

#include "AssetTypes.hpp"
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

        void LoadAssetFile(StringView path)
        {
            if (FileSystem::GetFileStatus(path).isDirectory)
            {
                for (const auto& entry : DirectoryEntries{path})
                {
                    LoadAssetFile(entry);
                }
            }

            String extension = Path::Extension(path);
            if (extension == FY_META_EXTENSION)
            {

            }
        }

        void LoadDirectoryPackage(StringView name, StringView path)
        {
            LoadAssetFile(path);
        }
    }

    void AssetManager::LoadPackage(StringView name, StringView path)
    {
        if (FileSystem::GetFileStatus(path).isDirectory)
        {
            LoadDirectoryPackage(name, path);
        }
    }
}
