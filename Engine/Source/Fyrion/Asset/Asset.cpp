#include "Asset.hpp"
#include "AssetTypes.hpp"

namespace Fyrion
{
    struct RenameTransactionAction : AssetTransactionAction
    {
        Asset* asset;
        String oldName;
        String newName;

        RenameTransactionAction(Asset* asset, const String& oldName, const String& newName)
            : asset(asset),
              oldName(oldName),
              newName(newName)
        {
        }

        void Undo() override
        {
            asset->SetName(oldName);
        }

        void Redo() override
        {
            asset->SetName(newName);
        }
    };


    void AssetDatabaseUpdatePath(Asset* asset, const StringView& oldPath, const StringView& newPath);

    void Asset::BuildPath()
    {
        if (directory != nullptr && !name.Empty())
        {
            SetPath(String().Append(directory->GetPath()).Append("/").Append(name));
        }
    }

    void AssetTransaction::AddRename(Asset* asset,StringView oldName, StringView newName)
    {
        actions.EmplaceBack(MakeShared<RenameTransactionAction>(asset, oldName, newName));
    }

    void AssetTransaction::Undo()
    {
        for(const auto& action : actions)
        {
            action->Undo();
        }
    }

    void AssetTransaction::Redo()
    {
        for(const auto& action : actions)
        {
            action->Redo();
        }
    }

    void Asset::SetPath(StringView p_path)
    {
        AssetDatabaseUpdatePath(this, path, p_path);
        path = p_path;
    }

    void Asset::SetName(StringView p_name)
    {
        name = p_name;
        BuildPath();
    }

    void Asset::SetDirectory(AssetDirectory* p_directory)
    {
        directory = p_directory;
        BuildPath();
    }

    bool Asset::IsParentOf(Asset* asset) const
    {
        if (asset == this) return false;

        if (directory != nullptr)
        {
            if (reinterpret_cast<usize>(asset->GetDirectory()) == reinterpret_cast<usize>(asset))
            {
                return true;
            }

            return directory->IsParentOf(asset);
        }
        return false;
    }

    void Asset::Rename(const StringView& newName, AssetTransaction* transaction)
    {
        if (transaction)
        {
            transaction->AddRename(this, GetName(), newName);
        }
        SetName(newName);
    }

    void Asset::Move(Asset* newDirectory, AssetTransaction* transaction)
    {
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {
    }
}
