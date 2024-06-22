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


    void InitAssetEditorActions()
    {
        Registry::Type<RenameAssetAction>();
    }
}
