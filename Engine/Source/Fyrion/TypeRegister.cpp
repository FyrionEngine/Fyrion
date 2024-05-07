#include "TypeRegister.hpp"

namespace Fyrion
{
    void RegisterCoreTypes();
    void RegisterResourceTypes();
    void RegisterResourceGraphTypes();
    void RegisterSceneType();
    void RegisterGraphicsTypes();

    void TypeRegister()
    {
        RegisterCoreTypes();
        RegisterResourceTypes();
        RegisterResourceGraphTypes();
        RegisterSceneType();
        RegisterGraphicsTypes();
    }
}