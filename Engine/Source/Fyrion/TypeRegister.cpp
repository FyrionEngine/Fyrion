#include "TypeRegister.hpp"

namespace Fyrion
{
    void RegisterCoreTypes();
    void RegisterIOTypes();
    void RegisterGraphicsTypes();
    void RegisterSceneTypes();

    void TypeRegister()
    {
        RegisterCoreTypes();
        RegisterIOTypes();
        RegisterGraphicsTypes();
        RegisterSceneTypes();
    }
}