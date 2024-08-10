#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    struct AssetIO;
    class Asset;
    class AssetDirectory;


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

        Asset*     prototype{};
        StringView absolutePath{};

        //generate a random UUID if field uuid is not provided
        bool generateUUID = true;
    };

    class FY_API AssetDatabase
    {
    public:
        static AssetDirectory* LoadFromDirectory(const StringView& name, const StringView& directory);
        static void            SaveOnDirectory(AssetDirectory* directoryAsset, const StringView& directoryPath);
        static void            SetCacheDirectory(const StringView& directory);
        static StringView      GetCacheDirectory();
        static void            GetUpdatedAssets(AssetDirectory* directoryAsset, Array<Asset*>& updatedAssets);
        static AssetDirectory* LoadFromPackage(const StringView& name, const StringView& pakFile, const StringView& binFile);
        static void            ImportAsset(AssetDirectory* directory, const StringView& path);
        static bool            CanReimportAsset(Asset* asset);
        static void            ReimportAsset(Asset* asset);
        static Asset*          FindById(const UUID& assetId);
        static Asset*          FindByPath(const StringView& path);
        static Span<Asset*>    FindAssetsByType(TypeID typeId);
        static Asset*          Create(TypeID typeId, const AssetCreation& assetCreation);
        static void            DestroyAssets();
        static void            EnableHotReload(bool enable);
        static void            WatchAsset(Asset* asset);

        template <typename T>
        static T* Create()
        {
            return static_cast<T*>(Create(GetTypeID<T>(), {}));
        }

        template <typename T>
        static T* Create(const AssetCreation& assetCreation)
        {
            return static_cast<T*>(Create(GetTypeID<T>(), assetCreation));
        }

        template <typename T>
        static T* FindById(const UUID& assetId)
        {
            return static_cast<T*>(FindById(assetId));
        }

        template <typename T>
        static T* FindByPath(const StringView& path)
        {
            return static_cast<T*>(FindByPath(path));
        }

        static void OnUpdate(f64 deltaTime);

    private:
        static void          LoadAssetFile(AssetDirectory* directory, const StringView& filePath);
        static void          SaveInfoJson(StringView file, Asset* asset);
        static ArchiveObject SaveInfo(ArchiveWriter& writer, Asset* asset, bool child = false);
        static void          SaveAsset(StringView file, Asset* asset);
        static void          LoadInfoJson(StringView file, Asset* asset);
        static void          LoadInfo(ArchiveReader& reader, ArchiveObject object, Asset* asset);
        static void          LoadAssetJson(StringView file, Asset* asset);
        static void          LoadAsset(ArchiveReader& reader, ArchiveObject object, Asset* asset);
        static void          QueueAssetImport(AssetIO* io, Asset* asset);
    };
}
