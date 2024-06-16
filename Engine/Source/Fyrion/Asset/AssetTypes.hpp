#pragma once
#include "Asset.hpp"

namespace Fyrion
{
    class AssetDirectory : public Asset
    {
    public:
        SubobjectList<Asset> GetChildren() const
        {
            return children;
        }

    private:
        SubobjectList<Asset> children;
    };
}
