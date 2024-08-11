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
          oldDirectory(asset->GetParent()),
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
        directory->AddChild(asset);
        asset->SetModified();
    }

    void MoveAssetAction::RegisterType(NativeTypeHandler<MoveAssetAction>& type)
    {
        type.Constructor<Asset*, AssetDirectory*>();
    }

    UpdateAssetAction::UpdateAssetAction(Asset* asset, Asset* newValue) : asset(asset)
    {
        JsonAssetWriter writer;
        currentStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(asset->GetType(), writer, asset));
        newStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(asset->GetType(), writer, newValue));
    }

    void UpdateAssetAction::Commit()
    {
        JsonAssetReader reader(newStrValue);
        Serialization::Deserialize(asset->GetType(), reader, reader.ReadObject(), asset);

        ImGui::ClearDrawData(asset);
        asset->SetModified();
    }
    void UpdateAssetAction::Rollback()
    {
        JsonAssetReader reader(currentStrValue);
        Serialization::Deserialize(asset->GetType(), reader, reader.ReadObject(), asset);

        ImGui::ClearDrawData(asset);

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
        Registry::Type<UpdateAssetAction>();
    }
}
