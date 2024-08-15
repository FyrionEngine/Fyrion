#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/UUID.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    class DirectoryAssetHandler;
}

namespace Fyrion
{
    class JsonAssetHandler;
    class AssetHandler;
    struct AssetIO;
    class Asset;

    struct AssetCreation
    {
        StringView name{};

        //selecting a different asset, persist the asset data as a child.
        //selecting a AssetDirectory, create the asset inside the directory.
        AssetHandler* parent{};

        DirectoryAssetHandler* directoryAsset{};

        //paths are build using directory + name, but in case of missing one of these info,
        //desired path can be used to lookup the asset.
        StringView desiredPath{};
    };

    class FY_API AssetManager
    {
    public:
        static DirectoryAssetHandler* LoadFromDirectory(const StringView& name, const StringView& directory);
        static void                   SaveOnDirectory(DirectoryAssetHandler* directoryAssetHandler, const StringView& directoryPath);
        static void                   SetDataDirectory(const StringView& directory);
        static StringView             GetDataDirectory();
        static void                   GetUpdatedAssets(DirectoryAssetHandler* directoryAssetHandler, Array<AssetHandler*>& updatedAssets);
        static DirectoryAssetHandler* LoadFromPackage(const StringView& name, const StringView& pakFile, const StringView& binFile);
        static void                   ImportAsset(DirectoryAssetHandler* directoryAssetHandler, const StringView& path);
        static bool                   CanReimportAsset(AssetHandler* assetHandler);
        static void                   ReimportAsset(AssetHandler* asset);
        static void                   QueueAssetImport(AssetIO* io, AssetHandler* assetHandler);
        static Asset*                 LoadById(const UUID& assetId);
        static Asset*                 LoadByPath(const StringView& path);
        static Span<Asset*>           FindAssetsByType(TypeID typeId);
        static AssetHandler*          FindHandlerByPath(const StringView& path);
        static Asset*                 Create(TypeHandler* typeHandler, const AssetCreation& assetCreation);
        static void                   DestroyAssets();
        static void                   EnableHotReload(bool enable);
        static void                   WatchAsset(AssetHandler* assetHandler);

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

        template <typename T>
        static T* FindHandlerByPath(const StringView& path)
        {
            return dynamic_cast<T*>(FindHandlerByPath(path));
        }

        static void OnUpdate(f64 deltaTime);

        friend class AssetHandler;

    private:
        static void LoadAssetFile(DirectoryAssetHandler* directoryAssetHandler, const StringView& filePath);
    };
}
