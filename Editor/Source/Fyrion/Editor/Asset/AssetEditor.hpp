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


    namespace AssetEditor
    {
        FY_API void             AddPackage(StringView directory);
        FY_API Span<AssetFile*> GetPackages();
        FY_API AssetFile*       CreateDirectory(AssetFile* parent);
        FY_API AssetFile*       CreateAsset(AssetFile* parent, TypeID typeId);
        FY_API void             Rename(AssetFile* assetFile, StringView newName);
        FY_API void             GetUpdatedAssets(Array<AssetFile*>& updatedAssets);
        FY_API void             SaveAssets(Span<AssetFile*> assetsToSave);
        FY_API void             DeleteAssets(Span<AssetFile*> assetFile);
        FY_API String           CreateUniqueName(AssetFile* parent, StringView desiredName);
        FY_API void             ImportAssets(Span<String> paths);
        FY_API void             FilterExtensions(Array<FileFilter>& extensions);
    }
}
