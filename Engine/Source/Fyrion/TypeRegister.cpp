#include "TypeRegister.hpp"

namespace Fyrion
{
    void RegisterCoreTypes();
    void RegisterGraphicsTypes();
    void RegisterSceneTypes();

    void TypeRegister()
    {
        RegisterCoreTypes();
        RegisterGraphicsTypes();
        RegisterSceneTypes();
    }
}