#include "TypeRegister.hpp"

namespace Fyrion
{
    void RegisterCoreTypes();
    void RegisterGraphicsTypes();

    void TypeRegister()
    {
        RegisterCoreTypes();
        RegisterGraphicsTypes();
    }
}