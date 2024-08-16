#include "SceneObjectAsset.hpp"

#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Scene/SceneManager.hpp"


namespace Fyrion
{
    SceneObject* SceneObjectAsset::GetObject()
    {
        return &object;
    }

    SceneObjectAsset* SceneObjectAsset::GetSceneObjectAsset()
    {
        return this;
    }

    void SceneObjectAsset::RegisterType(NativeTypeHandler<SceneObjectAsset>& type)
    {
        type.Attribute<AssetMeta>(AssetMeta{.displayName = "Scene"});
        type.Field<&SceneObjectAsset::object>("object");
    }
}
