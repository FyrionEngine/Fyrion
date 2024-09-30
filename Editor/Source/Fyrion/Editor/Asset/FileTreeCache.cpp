#include "FileTreeCache.hpp"

#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/FileTypes.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    void FileTreeCache::AddDirectory(StringView directory)
    {
        SharedPtr<FileTreeCacheNode> node = MakeShared<FileTreeCacheNode>();
        directories.EmplaceBack(node);
        UpdateNode(directory, node);
    }

    const FileTreeCacheNode* FileTreeCache::FindNode(StringView path) const
    {
        if (auto it = nodes.Find(path))
        {
            return it->second.Get();
        }

        return nullptr;
    }


    void FileTreeCache::UpdateNode(StringView path, SharedPtr<FileTreeCacheNode> node)
    {
        nodes.Insert(path, node);

        node->children.Clear();

        FileStatus status = FileSystem::GetFileStatus(path);

        node->absolutePath = path;
        node->fileName = Path::Name(path);
        node->hash = HashInt32(HashValue(path));
        node->isDirectory = status.isDirectory;

        for (const String& child : DirectoryEntries{path})
        {
            SharedPtr<FileTreeCacheNode> childrenNode = MakeShared<FileTreeCacheNode>();
            node->children.EmplaceBack(childrenNode);
            UpdateNode(child, childrenNode);
        }
    }
}
