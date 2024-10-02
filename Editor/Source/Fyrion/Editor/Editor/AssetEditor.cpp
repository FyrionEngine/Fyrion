#include "AssetEditor.hpp"

#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/FileTypes.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void AssetEditor::AddDirectory(StringView directory)
    {
        SharedPtr<AssetCache> node = MakeShared<AssetCache>();
        directories.EmplaceBack(node);
        UpdateCache(directory, node);
    }

    const AssetCache* AssetEditor::FindNode(StringView path) const
    {
        if (auto it = assets.Find(path))
        {
            return it->second.Get();
        }

        return nullptr;
    }

    String AssetEditor::CreateDirectory(StringView parentPath)
    {
        return {};
    }

    void AssetEditor::UpdateCache(StringView path, SharedPtr<AssetCache> cache)
    {
        assets.Insert(path, cache);

        cache->children.Clear();

        FileStatus status = FileSystem::GetFileStatus(path);

        cache->absolutePath = path;
        cache->fileName = Path::Name(path);
        cache->hash = HashInt32(HashValue(path));
        cache->isDirectory = status.isDirectory;

        for (const String& child : DirectoryEntries{path})
        {
            SharedPtr<AssetCache> childrenNode = MakeShared<AssetCache>();
            cache->children.EmplaceBack(childrenNode);
            UpdateCache(child, childrenNode);
        }
    }
}
