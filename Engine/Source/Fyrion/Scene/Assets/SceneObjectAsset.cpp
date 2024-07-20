#include "SceneObjectAsset.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"


namespace Fyrion
{
    void SceneObjectAsset::SetName(StringView p_name)
    {
        object.SetName(p_name);
        Asset::SetName(p_name);
    }

    void SceneObjectAsset::RegisterType(NativeTypeHandler<SceneObjectAsset>& type)
    {
        type.Field<&SceneObjectAsset::object>("object");
        type.Field<&SceneObjectAsset::renderGraph>("renderGraph");
    }
}
