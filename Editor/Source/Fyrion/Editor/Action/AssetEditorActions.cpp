#include "AssetEditorActions.hpp"
#include "Fyrion/Asset/Asset.hpp"

namespace Fyrion
{

    void RenameAssetAction::RegisterType(NativeTypeHandler<RenameAssetAction>& type)
    {
        type.Constructor<Asset*, StringView>();
    }

    RenameAssetAction::RenameAssetAction(Asset* asset, const StringView& newName) : asset(asset), oldName(asset->GetName()), newName(newName)
    {
    }

    void RenameAssetAction::Commit()
    {
        asset->SetName(newName);
    }

    void RenameAssetAction::Rollback()
    {
        asset->SetName(oldName);
    }

    MoveAssetAction::MoveAssetAction(Asset* asset, Asset* newDirectory)
        : asset(asset),
        oldDirectory(asset->GetDirectory()),
        newDirectory(newDirectory)
    {
    }

    void MoveAssetAction::Commit()
    {
        MoveToFolder(newDirectory);
    }

    void MoveAssetAction::Rollback()
    {
        MoveToFolder(oldDirectory);
    }

    void MoveAssetAction::MoveToFolder(Asset* directory)
    {
        //TODO
    }

    void MoveAssetAction::RegisterType(NativeTypeHandler<MoveAssetAction>& type)
    {
        type.Constructor<Asset*, Asset*>();
    }

    AssetCreationAction::AssetCreationAction(Asset* directory, TypeID typeId)
    {

    }

    void InitAssetEditorActions()
    {
        Registry::Type<RenameAssetAction>();
        Registry::Type<MoveAssetAction>();
    }
}
