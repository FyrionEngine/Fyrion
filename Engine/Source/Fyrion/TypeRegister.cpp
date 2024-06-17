#include "TypeRegister.hpp"

namespace Fyrion
{
    void RegisterCoreTypes();
    void RegisterAssetTypes();
    void RegisterSceneType();
    void RegisterGraphicsTypes();

    void TypeRegister()
    {
        RegisterCoreTypes();
        RegisterAssetTypes();
        RegisterSceneType();
        RegisterGraphicsTypes();
    }
}