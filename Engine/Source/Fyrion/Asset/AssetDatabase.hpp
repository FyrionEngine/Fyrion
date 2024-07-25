#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    class Asset;
    class AssetDirectory;

    class FY_API AssetDatabase
    {
    public:
        static AssetDirectory* LoadFromDirectory(const StringView& name, const StringView& directory);
        static void            SaveOnDirectory(AssetDirectory* directoryAsset, const StringView& directoryPath);
        static void            SetDataDirectory(const StringView& directory);
        static StringView      GetDataDirectory();
        static void            GetUpdatedAssets(AssetDirectory* directoryAsset, Array<Asset*>& updatedAssets);
        static AssetDirectory* LoadFromPackage(const StringView& name, const StringView& pakFile, const StringView& binFile);
        static Asset*          ImportAsset(AssetDirectory* directory, const StringView& path);
        static bool            CanReimportAsset(Asset* asset);
        static void            ReimportAsset(Asset* asset);
        static Asset*          FindById(const UUID& assetId);
        static Asset*          FindByPath(const StringView& path);
        static Span<Asset*>    FindAssetsByType(TypeID typeId);
        static Asset*          Create(TypeID typeId);
        static Asset*          Create(TypeID typeId, UUID uuid);
        static Asset*          CreateFromPrototype(TypeID typeId, Asset* prototype);
        static Asset*          CreateFromPrototype(TypeID typeId, Asset* prototype, UUID uuid);
        static void            Destroy(Asset* asset);
        static void            DestroyAssets();

        template <typename T>
        static T* Create()
        {
            return static_cast<T*>(Create(GetTypeID<T>()));
        }

        template <typename T>
        static T* Create(UUID uuid)
        {
            return static_cast<T*>(Create(GetTypeID<T>(), uuid));
        }

        template <typename T>
        static T* CreateFromPrototype(T* prototype)
        {
            return static_cast<T*>(CreateFromPrototype(GetTypeID<T>(), prototype));
        }

        template <typename T>
        static T* CreateFromPrototype(T* prototype, UUID uuid)
        {
            return static_cast<T*>(CreateFromPrototype(GetTypeID<T>(), prototype, uuid));
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

        static void          OnUpdate(f64 deltaTime);

    private:
        static void          LoadAssetFile(AssetDirectory* directory, const StringView& filePath);
        static Asset*        ReadAssetFile(const StringView& path);
        static ArchiveObject SerializeAsset(ArchiveWriter& writer, Asset* asset);
        static Asset*        DeserializeAsset(ArchiveReader& reader, ArchiveObject object);
    };
}
