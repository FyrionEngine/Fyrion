#include "AssetTypes.hpp"

#include "AssetDatabase.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/IO/FileSystem.hpp"


namespace Fyrion
{
    void AssetIO::RegisterType(NativeTypeHandler<AssetIO>& type)
    {
        type.Function<&AssetIO::GetImportExtensions>("GetImportExtensions");
        type.Function<&AssetIO::ImportAsset>("ImportAsset");
    }

    void AssetDirectory::RegisterType(NativeTypeHandler<AssetDirectory>& type)
    {
        type.Field<&AssetDirectory::children>("children");
    }

    void AssetDirectory::BuildPath()
    {
        Asset::BuildPath();
        for (Asset* child : children)
        {
            child->BuildPath();
        }
    }

    void AssetDirectory::OnActiveChanged()
    {
        for (Asset* child : children)
        {
            child->SetActive(IsActive());
        }
    }

    StringView AssetDirectory::GetDisplayName() const
    {
        return "Folder";
    }

    void AssetDirectory::AddChild(Asset* child)
    {
        if (child->GetDirectory() != nullptr)
        {
            child->GetDirectory()->RemoveChild(child);
        }
        child->directory = this;
        child->BuildPath();
        children.EmplaceBack(child);
    }

    void AssetDirectory::RemoveChild(Asset* child)
    {
        if (Asset** it = FindFirst(children.begin(), children.end(), child))
        {
            children.Erase(it);
        }
    }

    Span<Asset*> AssetDirectory::GetChildren()
    {
        return children;
    }

    bool AssetDirectory::IsModified() const
    {
        if (!IsActive() && loadedVersion == 0)
        {
            return false;
        }
        return currentVersion != loadedVersion;
    }

    bool AssetDirectory::HasChild(const StringView& childAbsolutePath) const
    {
        for (Asset* child : children)
        {
            if (child->absolutePath == childAbsolutePath)
            {
                return true;
            }
        }
        return false;
    }

    void UIFontAsset::RegisterType(NativeTypeHandler<UIFontAsset>& type)
    {
    }

    Span<StringView> UIFontAssetIO::GetImportExtensions()
    {
        return {extensions, 2};
    }

    Asset* UIFontAssetIO::CreateAsset()
    {
        return AssetDatabase::Create<UIFontAsset>();
    }

    void UIFontAssetIO::ImportAsset(StringView path, Asset* asset)
    {
        UIFontAsset* fontAsset = asset->Cast<UIFontAsset>();
        fontAsset->fontBytes = FileSystem::ReadFileAsByteArray(path);
    }

    void RegisterAssetTypes()
    {
        Registry::Type<AssetIO>();
        Registry::Type<Asset>();
        Registry::Type<AssetDirectory>();
        Registry::Type<UIFontAsset>();
        Registry::Type<UIFontAssetIO>();
    }
}
