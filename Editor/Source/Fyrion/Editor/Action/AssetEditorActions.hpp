#pragma once
#include "EditorAction.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    class AssetDirectory;
}

namespace Fyrion
{
    class Asset;

    class RenameAssetAction : public EditorAction
    {
    public:
        FY_BASE_TYPES(EditorAction);

        RenameAssetAction(Asset* asset, const StringView& newName);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<RenameAssetAction>& type);

    private:
        Asset* asset;
        String oldName;
        String newName;
    };


    class MoveAssetAction : public EditorAction
    {
    public:
        FY_BASE_TYPES(EditorAction);

        MoveAssetAction(Asset* asset, AssetDirectory* newDirectory);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<MoveAssetAction>& type);

    private:
        void MoveToFolder(AssetDirectory* directory) const;

        Asset*          asset;
        AssetDirectory* oldDirectory;
        AssetDirectory* newDirectory;
    };


    class AssetCreateAction : public EditorAction
    {
    public:
        FY_BASE_TYPES(EditorAction);

        AssetCreateAction(AssetDirectory* directory, TypeID typeId);

        void Commit() override;
        void Rollback() override;

        Asset* GetNewAsset() const
        {
            return newAsset;
        }

        static void RegisterType(NativeTypeHandler<AssetCreateAction>& type);

        ~AssetCreateAction() override;

    private:
        Asset* newAsset;
    };

    class AssetDeleteAction : public EditorAction
    {
    public:
        FY_BASE_TYPES(EditorAction);
        AssetDeleteAction(Asset* asset);

        void Commit() override;
        void Rollback() override;

        ~AssetDeleteAction() override;

        static void RegisterType(NativeTypeHandler<AssetDeleteAction>& type);

    private:
        Asset* asset;
    };
}
