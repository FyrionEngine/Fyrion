#include "AssetEditorActions.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Asset/AssetSerialization.hpp"
#include "Fyrion/ImGui/ImGui.hpp"

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

    void MoveAssetAction::MoveToFolder(AssetDirectory* directory) const
    {
        directory->AddChild(asset);
        asset->SetModified();
    }

    void MoveAssetAction::RegisterType(NativeTypeHandler<MoveAssetAction>& type)
    {
        type.Constructor<Asset*, AssetDirectory*>();
    }

    AssetCreateAction::AssetCreateAction(AssetDirectory* directory, TypeID typeId)
    {
        newAsset = AssetDatabase::Create(typeId, UUID::RandomUUID());
        newAsset->SetName(String("New ").Append(newAsset->GetDisplayName()));
        newAsset->SetExtension(FY_ASSET_EXTENSION);
        directory->AddChild(newAsset);
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
        if (!newAsset->IsActive() && !newAsset->IsModified() && newAsset->GetAbsolutePath().Empty())
        {
            newAsset->GetDirectory()->RemoveChild(newAsset);
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

    UpdateAssetAction::UpdateAssetAction(Asset* asset, Asset* newValue) : asset(asset)
    {
        JsonAssetWriter writer;
        currentStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(asset->GetAssetType(), writer, asset));
        newStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(asset->GetAssetType(), writer, newValue));
    }

    void UpdateAssetAction::Commit()
    {
        JsonAssetReader reader(newStrValue);
        Serialization::Deserialize(asset->GetAssetType(), reader, reader.ReadObject(), asset);

        ImGui::ClearTextData();
        asset->SetModified();
    }
    void UpdateAssetAction::Rollback()
    {
        JsonAssetReader reader(currentStrValue);
        Serialization::Deserialize(asset->GetAssetType(), reader, reader.ReadObject(), asset);

        ImGui::ClearTextData();

        asset->SetModified();
    }

    void UpdateAssetAction::RegisterType(NativeTypeHandler<UpdateAssetAction>& type)
    {
        type.Constructor<Asset*, Asset*>();
    }

    void InitAssetEditorActions()
    {
        Registry::Type<RenameAssetAction>();
        Registry::Type<MoveAssetAction>();
        Registry::Type<AssetCreateAction>();
        Registry::Type<AssetDeleteAction>();
        Registry::Type<UpdateAssetAction>();
    }
}
