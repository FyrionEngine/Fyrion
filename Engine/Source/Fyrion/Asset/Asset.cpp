#include "Asset.hpp"
#include <algorithm>

#include "AssetDatabase.hpp"

namespace Fyrion
{
    void Subobject::Add(Asset* asset)
    {
        FY_ASSERT(asset, "asset is null, field not initialized");
        FY_ASSERT(!asset->subobjectOf, "asset is already a subobject");

        asset->subobjectOf = this;
        assets.EmplaceBack(asset);
    }


    void Subobject::Remove(Asset* asset)
    {
        FY_ASSERT(asset, "asset is null, field not initialized");
        if (const auto it = std::find(assets.begin(), assets.end(), asset); it != assets.end())
        {
            assets.Erase(it);
        }

    }

    usize Subobject::Count() const
    {
        usize total = assets.Size();
        if (asset->GetPrototype() != nullptr && field != nullptr)
        {
            Subobject& prototype =  field->GetValueAs<Subobject>(asset->GetPrototype());
            total += prototype.Count();
        }
        return total;
    }

    void Subobject::Get(Span<Asset*> retAssets) const
    {
        GetTo(retAssets, 0);
    }

    void Subobject::GetTo(Span<Asset*> retAssets, usize pos) const
    {

        if (asset->GetPrototype() != nullptr && field != nullptr)
        {
            Subobject& prototype =  field->GetValueAs<Subobject>(asset->GetPrototype());
            prototype.GetTo(retAssets, pos);
        }

        for (Asset* asset : assets)
        {
            retAssets[pos++] = asset;
        }
    }

    void Subobject::RegisterType(NativeTypeHandler<Subobject>& type)
    {
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {
    }

    void AssetField::RegisterType(NativeTypeHandler<AssetField>& type)
    {
        type.Field<&AssetField::asset>("asset");
    }
}
