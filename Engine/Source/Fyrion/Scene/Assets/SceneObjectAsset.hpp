#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Scene/Component.hpp"


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

        StringView GetDisplayName() const override
        {
            return "Scene";
        }

        static void RegisterType(NativeTypeHandler<SceneObjectAsset>& type);

    private:
        Subobject<SceneObjectAsset> children;
        Subobject<Component> components;
    };

}
