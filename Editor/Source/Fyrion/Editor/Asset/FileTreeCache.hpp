#pragma once
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"


namespace Fyrion
{
    struct FileTreeCacheNode
    {
        u32                                 hash;
        String                              fileName;
        String                              absolutePath;
        bool                                isDirectory;
        Array<SharedPtr<FileTreeCacheNode>> children;
    };


    class FileTreeCache
    {
    public:
        void AddDirectory(StringView directory);

        decltype(auto) GetDirectories() const
        {
            return directories;
        }

        const FileTreeCacheNode* FindNode(StringView path) const;

    private:
        Array<SharedPtr<FileTreeCacheNode>>           directories;
        HashMap<String, SharedPtr<FileTreeCacheNode>> nodes;

        void UpdateNode(StringView path, SharedPtr<FileTreeCacheNode> node);
    };
}
