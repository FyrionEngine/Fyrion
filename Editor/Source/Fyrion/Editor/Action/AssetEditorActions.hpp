#pragma once
#include "EditorAction.hpp"
#include "Fyrion/Core/Registry.hpp"


namespace Fyrion
{
    class Asset;

    class RenameAssetAction final : public EditorAction
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
}
