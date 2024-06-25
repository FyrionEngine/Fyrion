#include "Assets/SceneObjectAsset.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RegisterSceneType()
    {
        Registry::Type<SceneObjectAsset>();
    }
}
