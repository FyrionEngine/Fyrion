#include "TypeRegister.hpp"

namespace Fyrion
{
    void RegisterCoreTypes();
    void RegisterResourceTypes();
    void RegisterSceneType();
    void RegisterGraphicsTypes();

    void TypeRegister()
    {
        RegisterCoreTypes();
        RegisterResourceTypes();
        RegisterSceneType();
        RegisterGraphicsTypes();
    }
}