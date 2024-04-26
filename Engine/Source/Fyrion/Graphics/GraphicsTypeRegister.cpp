#include "Fyrion/Core/Registry.hpp"
#include "GraphicsTypes.hpp"

namespace Fyrion
{
    void RegisterGraphicsTypes()
    {
        Registry::Type<ShaderStageInfo>();
        Registry::Type<Array<ShaderStageInfo>>();
        Registry::Type<ShaderInfo>();
    }
}
