#pragma once
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    struct AssetFile
    {
        u32    hash;
        String fileName;
        String extension;
        String absolutePath;
        bool   isDirectory;
        UUID   assetId;

        u64 currentVersion;
        u64 persistedVersion;

        Array<AssetFile*> children;

        bool IsDirty() const;
    };


    class AssetEditor
    {
    public:
        void AddPackage(StringView directory);

        decltype(auto) GetDirectories() const
        {
            return directories;
        }

        const AssetFile* FindNode(StringView path) const;

        String CreateDirectory(StringView parentPath);
        void   Rename(AssetFile* assetFile, StringView newName);
        void   GetUpdatedAssets(Array<AssetFile*>& updatedAssets) const;
        void   SaveAssets(Span<AssetFile*> assetsToSave);

        ~AssetEditor();

    private:
        Array<AssetFile*>           directories;
        HashMap<String, AssetFile*> assets;

        void UpdateCache(StringView path, AssetFile* assetFile);
    };
}
