#pragma once
#include "EditorAction.hpp"
#include "Fyrion/Core/Registry.hpp"


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

        MoveAssetAction(Asset* asset, Asset* newDirectory);

        void Commit() override;
        void Rollback() override;

        static void RegisterType(NativeTypeHandler<MoveAssetAction>& type);

    private:

        void MoveToFolder(Asset* directory);

        Asset* asset;
        Asset* oldDirectory;
        Asset* newDirectory;
    };


    class AssetCreationAction : public EditorAction
    {
    public:
        FY_BASE_TYPES(AssetCreationAction);

        AssetCreationAction(Asset* directory, TypeID typeId);

        AssetCreationAction();

    private:

    };
}
