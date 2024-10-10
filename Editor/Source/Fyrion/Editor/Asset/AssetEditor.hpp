#pragma once
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/UUID.hpp"
#include "Fyrion/Core/Span.hpp"


namespace Fyrion
{
    struct AssetImporter;

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

        AssetFile* parent;

        bool active = true;

        bool IsDirty() const;
    };


    class AssetEditor
    {
    public:
        void Init();
        void AddPackage(StringView directory);

        decltype(auto) GetDirectories() const
        {
            return directories;
        }

        const AssetFile* FindNode(StringView path) const;

        AssetFile* CreateDirectory(AssetFile* parent);
        AssetFile* CreateAsset(AssetFile* parent, TypeID typeId);
        void       Rename(AssetFile* assetFile, StringView newName);
        void       GetUpdatedAssets(Array<AssetFile*>& updatedAssets) const;
        void       SaveAssets(Span<AssetFile*> assetsToSave);
        void       DeleteAssets(Span<AssetFile*> assetFile);
        String     CreateUniqueName(AssetFile* parent, StringView desiredName);

        void       ImportAssets(Span<String> paths);
        void       FilterExtensions(Array<FileFilter>& extensions);

        ~AssetEditor();

    private:
        Array<AssetFile*>           directories;
        HashMap<String, AssetFile*> assets;

        Array<AssetImporter*> importers;
        HashMap<String, AssetImporter*> extensionImporters;

        void UpdateCache(StringView path, AssetFile* assetFile);
    };
}
