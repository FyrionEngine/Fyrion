#include "TypeRegister.hpp"

namespace Fyrion
{
    void RegisterCoreTypes();
    void RegisterResourceTypes();
    void RegisterResourceGraphTypes();
    void RegisterSceneType();
    void RegisterGraphicsTypes();
    void RegisterAssetTypes();

    void TypeRegister()
    {
        RegisterCoreTypes();
        RegisterResourceTypes();
        RegisterResourceGraphTypes();
        RegisterSceneType();
        RegisterGraphicsTypes();
        RegisterAssetTypes();
    }
}