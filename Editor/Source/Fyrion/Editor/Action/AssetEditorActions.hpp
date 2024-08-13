#pragma once
#include "EditorAction.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    class Asset;
    class AssetInfo;

    class RenameAssetAction : public EditorAction
    {
    public:
        FY_BASE_TYPES(EditorAction);

        RenameAssetAction(AssetInfo* assetInfo, const StringView& newName);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<RenameAssetAction>& type);

    private:
        AssetInfo* assetInfo;
        String     oldName;
        String     newName;
    };


    class MoveAssetAction : public EditorAction
    {
    public:
        FY_BASE_TYPES(EditorAction);

        MoveAssetAction(AssetInfo* assetInfo, AssetInfo* newDirectory);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<MoveAssetAction>& type);

    private:
        void MoveToFolder(AssetInfo* directory) const;

        AssetInfo* assetInfo;
        AssetInfo* oldDirectory;
        AssetInfo* newDirectory;
    };

    struct UpdateAssetAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        Asset* asset;
        String currentStrValue;
        String newStrValue;

        UpdateAssetAction(Asset* assetInfo, Asset* newValue);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<UpdateAssetAction>& type);
    };
}
