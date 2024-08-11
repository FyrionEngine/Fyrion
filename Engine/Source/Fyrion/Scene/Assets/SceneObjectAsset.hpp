#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Scene/Component.hpp"
#include "Fyrion/Scene/SceneObject.hpp"
#include "Fyrion/Scene/SceneTypes.hpp"


namespace Fyrion
{
    class RenderGraphAsset;

    class FY_API SceneObjectAsset : public Asset, public SceneObjectAssetProvider
    {
    public:
        FY_BASE_TYPES(Asset, SceneObjectAssetProvider);

        SceneObject*      GetObject();
        SceneObjectAsset* GetSceneObjectAsset() override;

        static void       RegisterType(NativeTypeHandler<SceneObjectAsset>& type);

    private:
        SceneObject object{this};
    };
}
