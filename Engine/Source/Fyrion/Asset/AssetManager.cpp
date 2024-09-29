#include "AssetManager.hpp"

#include "Fyrion/IO/FileSystem.hpp"


namespace Fyrion
{

    void AssetManagerLoadDirectoryPackage(StringView name, StringView path)
    {

    }

    void AssetManager::LoadPackage(StringView name, StringView path)
    {
        if (FileSystem::GetFileStatus(path).isDirectory)
        {
            AssetManagerLoadDirectoryPackage(name, path);
        }
    }
}
