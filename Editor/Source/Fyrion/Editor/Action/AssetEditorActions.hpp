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
        void MoveToFolder(Asset* directory) const;

        Asset* asset;
        Asset* oldDirectory;
        Asset* newDirectory;
    };

    struct UpdateAssetAction : EditorAction
    {
        FY_BASE_TYPES(EditorAction);

        Asset* asset;
        String currentStrValue;
        String newStrValue;

        UpdateAssetAction(Asset* asset, Asset* newValue);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<UpdateAssetAction>& type);
    };
}
