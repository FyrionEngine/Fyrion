#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/UUID.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    class DirectoryAsset;
    class AssetInfo;
    struct AssetIO;
    class Asset;

    struct AssetCreation
    {
        UUID       uuid{};
        StringView name{};

        //selecting a AssetDirectory, create the asset inside the directory.
        //selecting a different asset, persist the asset data as a child.
        Asset* parent{};

        //paths are build using directory + name, but in case of missing one of these info,
        //desired path can be used to lookup the asset.
        StringView desiredPath{};

        //AssetInfo* prototype{};
        StringView absolutePath{};

        bool generateUUID = true;
    };

    class FY_API AssetManager
    {
    public:
        static DirectoryAsset* LoadFromDirectory(const StringView& name, const StringView& directory);
        static void            SaveOnDirectory(DirectoryAsset* directoryAsset, const StringView& directoryPath);
        static void            SetCacheDirectory(const StringView& directory);
        static StringView      GetCacheDirectory();
        static void            GetUpdatedAssets(DirectoryAsset* directoryAsset, Array<AssetInfo*>& updatedAssets);
        static DirectoryAsset* LoadFromPackage(const StringView& name, const StringView& pakFile, const StringView& binFile);
        static void            ImportAsset(DirectoryAsset* directory, const StringView& path);
        static bool            CanReimportAsset(AssetInfo* assetInfo);
        static void            ReimportAsset(AssetInfo* asset);
        static Asset*          LoadAsset(AssetInfo* assetInfo);
        static void            UnloadAsset(AssetInfo* assetInfo);
        static Asset*          LoadById(const UUID& assetId);
        static Asset*          LoadByPath(const StringView& path);
        static Span<Asset*>    FindAssetsByType(TypeID typeId);
        static Asset*          Create(TypeHandler* typeHandler, const AssetCreation& assetCreation);
        static void            DestroyAssets();
        static void            EnableHotReload(bool enable);
        static void            WatchAsset(AssetInfo* assetInfo);

        template <typename T>
        static T* Create(const AssetCreation& assetCreation = {})
        {
            return static_cast<T*>(Create(Registry::FindType<T>(), assetCreation));
        }

        template <typename T>
        static T* LoadById(const UUID& assetId)
        {
            return static_cast<T*>(LoadById(assetId));
        }

        template <typename T>
        static T* LoadByPath(const StringView& path)
        {
            return static_cast<T*>(LoadByPath(path));
        }

        static void OnUpdate(f64 deltaTime);

    private:
        static AssetInfo*    CreateAssetInfo();
        static void          LoadAssetFile(DirectoryAsset* directory, const StringView& filePath);
        static void          SaveInfoJson(StringView file, AssetInfo* assetInfo);
        static ArchiveObject SaveInfo(ArchiveWriter& writer, AssetInfo* assetInfo, bool isChild = false);
        static void          SaveAssetJson(StringView file, Asset* asset);
        static bool          LoadInfoJson(StringView file, AssetInfo* asset);
        static void          LoadInfo(ArchiveReader& reader, ArchiveObject object, AssetInfo* assetItem);
        static void          LoadAssetJson(StringView file, Asset* asset);
        static void          LoadAsset(ArchiveReader& reader, ArchiveObject object, Asset* asset);
        static void          QueueAssetImport(AssetIO* io, AssetInfo* assetInfo);
    };
}
