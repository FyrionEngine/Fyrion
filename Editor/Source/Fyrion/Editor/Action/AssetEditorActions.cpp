#include "AssetEditorActions.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"

namespace Fyrion
{
    void RenameAssetAction::RegisterType(NativeTypeHandler<RenameAssetAction>& type)
    {
        type.Constructor<Asset*, StringView>();
    }

    RenameAssetAction::RenameAssetAction(Asset* asset, const StringView& newName) : asset(asset), oldName(asset->GetName()), newName(newName) {}

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
          newDirectory(newDirectory) {}

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

    AssetCreationAction::AssetCreationAction(AssetDirectory* directory, TypeID typeId)
    {
        newAsset = AssetDatabase::Create(typeId);
        newAsset->SetName("New Folder");
        newAsset->SetDirectory(directory);
    }

    void AssetCreationAction::Commit()
    {
        newAsset->SetActive(true);
    }

    void AssetCreationAction::Rollback()
    {
        newAsset->SetActive(false);
    }

    AssetCreationAction::~AssetCreationAction()
    {
        if (!newAsset->IsActive())
        {
            AssetDatabase::Destroy(newAsset);
        }
    }

    void AssetCreationAction::RegisterType(NativeTypeHandler<AssetCreationAction>& type)
    {
        type.Constructor<AssetDirectory*, TypeID>();
    }

    void InitAssetEditorActions()
    {
        Registry::Type<RenameAssetAction>();
        Registry::Type<MoveAssetAction>();
        Registry::Type<AssetCreationAction>();
    }
}
