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

    MoveAssetAction::MoveAssetAction(Asset* asset, AssetDirectory* newDirectory)
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

    void MoveAssetAction::MoveToFolder(Asset* directory) const
    {
        asset->SetDirectory(directory);
    }

    void MoveAssetAction::RegisterType(NativeTypeHandler<MoveAssetAction>& type)
    {
        type.Constructor<Asset*, AssetDirectory*>();
    }

    AssetCreateAction::AssetCreateAction(AssetDirectory* directory, TypeID typeId)
    {
        newAsset = AssetDatabase::Create(typeId);
        newAsset->SetName("New Folder");
        newAsset->SetDirectory(directory);
    }

    void AssetCreateAction::Commit()
    {
        newAsset->SetActive(true);
    }

    void AssetCreateAction::Rollback()
    {
        newAsset->SetActive(false);
    }

    AssetCreateAction::~AssetCreateAction()
    {
        if (!newAsset->IsActive())
        {
            AssetDatabase::Destroy(newAsset);
        }
    }

    void AssetCreateAction::RegisterType(NativeTypeHandler<AssetCreateAction>& type)
    {
        type.Constructor<AssetDirectory*, TypeID>();
    }

    AssetDeleteAction::AssetDeleteAction(Asset* asset) : asset(asset) {}

    void AssetDeleteAction::Commit()
    {
        asset->SetActive(false);
    }

    void AssetDeleteAction::Rollback()
    {
        asset->SetActive(true);
    }

    AssetDeleteAction::~AssetDeleteAction()
    {
        if (!asset->IsActive())
        {
            AssetDatabase::Destroy(asset);
        }
    }

    void AssetDeleteAction::RegisterType(NativeTypeHandler<AssetDeleteAction>& type)
    {
        type.Constructor<Asset*>();
    }

    void InitAssetEditorActions()
    {
        Registry::Type<RenameAssetAction>();
        Registry::Type<MoveAssetAction>();
        Registry::Type<AssetCreateAction>();
        Registry::Type<AssetDeleteAction>();
    }
}
