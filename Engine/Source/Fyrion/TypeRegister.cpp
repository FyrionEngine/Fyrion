#include "TypeRegister.hpp"

namespace Fyrion
{
    void RegisterCoreTypes();
    void RegisterSceneType();
    void RegisterGraphicsTypes();

    void TypeRegister()
    {
        RegisterCoreTypes();
        RegisterSceneType();
        RegisterGraphicsTypes();
    }
}