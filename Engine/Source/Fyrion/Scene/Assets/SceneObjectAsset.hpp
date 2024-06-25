#pragma once
#include "Fyrion/Asset/Asset.hpp"


namespace Fyrion
{

    class FY_API SceneObjectAsset : public Asset
    {
    public:

        FY_BASE_TYPES(Asset);

        Subobject<SceneObjectAsset>& GetChildren()
        {
            return children;
        }

        StringView GetDisplayName() override
        {
            return "Scene";
        }

        static void RegisterType(NativeTypeHandler<SceneObjectAsset>& type);

    private:
        Subobject<SceneObjectAsset> children;
    };

}
