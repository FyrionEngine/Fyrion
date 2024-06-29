#include "SceneObjectAsset.hpp"


namespace Fyrion
{
    void SceneObjectAsset::RegisterType(NativeTypeHandler<SceneObjectAsset>& type)
    {
        type.Field<&SceneObjectAsset::object>("object");
    }
}
