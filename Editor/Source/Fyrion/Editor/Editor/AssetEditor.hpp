#pragma once
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"


namespace Fyrion
{
    struct AssetCache
    {
        u32                          hash;
        String                       fileName;
        String                       absolutePath;
        bool                         isDirectory;
        Array<SharedPtr<AssetCache>> children;
    };


    class AssetEditor
    {
    public:
        void AddDirectory(StringView directory);

        decltype(auto) GetDirectories() const
        {
            return directories;
        }

        const AssetCache* FindNode(StringView path) const;

        String CreateDirectory(StringView parentPath);

    private:
        Array<SharedPtr<AssetCache>>           directories;
        HashMap<String, SharedPtr<AssetCache>> assets;

        void UpdateCache(StringView path, SharedPtr<AssetCache> cache);
    };
}
