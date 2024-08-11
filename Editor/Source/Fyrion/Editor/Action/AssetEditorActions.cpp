#include "AssetEditorActions.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Asset/AssetManager.hpp"
#include "Fyrion/Asset/AssetSerialization.hpp"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{
    void RenameAssetAction::RegisterType(NativeTypeHandler<RenameAssetAction>& type)
    {
        type.Constructor<AssetInfo*, StringView>();
    }

    RenameAssetAction::RenameAssetAction(AssetInfo* assetInfo, const StringView& newName) : assetInfo(assetInfo), oldName(assetInfo->GetName()), newName(newName) {}

    void RenameAssetAction::Commit()
    {
        assetInfo->SetName(newName);
    }

    void RenameAssetAction::Rollback()
    {
        assetInfo->SetName(oldName);
    }

    MoveAssetAction::MoveAssetAction(AssetInfo* assetInfo, DirectoryInfo* newDirectory)
        : assetInfo(assetInfo),
          oldDirectory(dynamic_cast<DirectoryInfo*>(assetInfo->GetParent())),
          newDirectory(newDirectory) {}

    void MoveAssetAction::Commit()
    {
        MoveToFolder(newDirectory);
    }

    void MoveAssetAction::Rollback()
    {
        MoveToFolder(oldDirectory);
    }

    void MoveAssetAction::MoveToFolder(DirectoryInfo* directory) const
    {
        // directory->AddChild(asset);
        // asset->SetModified();
    }

    void MoveAssetAction::RegisterType(NativeTypeHandler<MoveAssetAction>& type)
    {
        type.Constructor<AssetInfo*, DirectoryInfo*>();
    }

    UpdateAssetAction::UpdateAssetAction(Asset* asset, Asset* newValue) : asset(asset)
    {
        JsonAssetWriter writer;
        currentStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(asset->GetInfo()->GetType(), writer, asset));
        newStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(asset->GetInfo()->GetType(), writer, newValue));
    }

    void UpdateAssetAction::Commit()
    {
        JsonAssetReader reader(newStrValue);
        Serialization::Deserialize(asset->GetInfo()->GetType(), reader, reader.ReadObject(), asset);

        ImGui::ClearDrawData(asset);
        asset->GetInfo()->SetModified();
    }
    void UpdateAssetAction::Rollback()
    {
        JsonAssetReader reader(currentStrValue);
        Serialization::Deserialize(asset->GetInfo()->GetType(), reader, reader.ReadObject(), asset);

        ImGui::ClearDrawData(asset);

        asset->GetInfo()->SetModified();
    }

    void UpdateAssetAction::RegisterType(NativeTypeHandler<UpdateAssetAction>& type)
    {
        type.Constructor<Asset*, Asset*>();
    }

    void InitAssetEditorActions()
    {
        Registry::Type<RenameAssetAction>();
        Registry::Type<MoveAssetAction>();
        Registry::Type<UpdateAssetAction>();
    }
}
