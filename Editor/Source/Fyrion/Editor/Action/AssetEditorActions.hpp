#pragma once
#include "EditorAction.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    class Asset;
    class AssetHandler;

    class RenameAssetAction : public EditorAction
    {
    public:
        FY_BASE_TYPES(EditorAction);

        RenameAssetAction(AssetHandler* assetHandler, const StringView& newName);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<RenameAssetAction>& type);

    private:
        AssetHandler* assetHandler;
        String     oldName;
        String     newName;
    };


    class MoveAssetAction : public EditorAction
    {
    public:
        FY_BASE_TYPES(EditorAction);

        MoveAssetAction(AssetHandler* assetHandler, AssetHandler* newDirectory);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<MoveAssetAction>& type);

    private:
        void MoveToFolder(AssetHandler* directory) const;

        AssetHandler* assetHandler;
        AssetHandler* oldDirectory;
        AssetHandler* newDirectory;
    };

    struct UpdateAssetAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        Asset* asset;
        String currentStrValue;
        String newStrValue;

        UpdateAssetAction(Asset* assetHandler, Asset* newValue);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<UpdateAssetAction>& type);
    };
}
