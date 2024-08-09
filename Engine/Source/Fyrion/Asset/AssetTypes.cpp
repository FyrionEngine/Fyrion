#include "AssetTypes.hpp"

#include "AssetDatabase.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


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

    // void AssetDirectory::OnActiveChanged()
    // {
    //     for (Asset* child : children)
    //     {
    //         child->SetActive(IsActive());
    //     }
    // }

    StringView AssetDirectory::GetDisplayName() const
    {
        return "Folder";
    }

    bool AssetDirectory::IsModified() const
    {
        return false;
    }

    void AssetDirectory::OnCreated()
    {
        AssetDirectory* parentDirectory = dynamic_cast<AssetDirectory*>(GetParent());
        FY_ASSERT(parentDirectory, "directories must have parent directories");
        if (parentDirectory)
        {
            absolutePath = Path::Join(parentDirectory->GetAbsolutePath(), GetName());
            FileSystem::CreateDirectory(absolutePath);
        }
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
