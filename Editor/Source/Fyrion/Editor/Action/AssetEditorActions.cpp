#include "AssetEditorActions.hpp"
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Asset/AssetHandler.hpp"
#include "Fyrion/Asset/AssetSerialization.hpp"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{
    void RenameAssetAction::RegisterType(NativeTypeHandler<RenameAssetAction>& type)
    {
    }

    RenameAssetAction::RenameAssetAction(AssetHandler* assetHandler, const StringView& newName) : assetHandler(assetHandler), oldName(assetHandler->GetName()), newName(newName) {}

    void RenameAssetAction::Commit()
    {
        assetHandler->SetName(newName);
    }

    void RenameAssetAction::Rollback()
    {
        assetHandler->SetName(oldName);
    }

    MoveAssetAction::MoveAssetAction(AssetHandler* assetHandler, AssetHandler* newDirectory)
        : assetHandler(assetHandler),
          oldDirectory(assetHandler->GetParent()),
          newDirectory(newDirectory) {}

    void MoveAssetAction::Commit()
    {
        MoveToFolder(newDirectory);
    }

    void MoveAssetAction::Rollback()
    {
        MoveToFolder(oldDirectory);
    }

    void MoveAssetAction::MoveToFolder(AssetHandler* directory) const
    {
        int a=  0;
         // directory->AddChild(assetHandler);
         // assetHandler->SetModified();
    }

    void MoveAssetAction::RegisterType(NativeTypeHandler<MoveAssetAction>& type)
    {
    }

    UpdateAssetAction::UpdateAssetAction(Asset* asset, Asset* newValue) : asset(asset)
    {
        JsonAssetWriter writer;
        currentStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(asset->GetHandler()->GetType(), writer, asset));
        newStrValue = JsonAssetWriter::Stringify(Serialization::Serialize(asset->GetHandler()->GetType(), writer, newValue));
    }

    void UpdateAssetAction::Commit()
    {
        JsonAssetReader reader(newStrValue);
        Serialization::Deserialize(asset->GetHandler()->GetType(), reader, reader.ReadObject(), asset);

        ImGui::ClearDrawData(asset);
        asset->SetModified();
    }
    void UpdateAssetAction::Rollback()
    {
        JsonAssetReader reader(currentStrValue);
        Serialization::Deserialize(asset->GetHandler()->GetType(), reader, reader.ReadObject(), asset);

        ImGui::ClearDrawData(asset);

        asset->SetModified();
    }

    void UpdateAssetAction::RegisterType(NativeTypeHandler<UpdateAssetAction>& type)
    {
    }

    void InitAssetEditorActions()
    {
        Registry::Type<RenameAssetAction>();
        Registry::Type<MoveAssetAction>();
        Registry::Type<UpdateAssetAction>();
    }
}
