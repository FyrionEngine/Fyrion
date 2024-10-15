#include "Asset.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void RegisterIOTypes()
    {
        Registry::Type<Asset>();
    }
}
