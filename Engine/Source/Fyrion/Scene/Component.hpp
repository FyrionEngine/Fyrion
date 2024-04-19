#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    class SceneObject;

    class FY_API Component
    {
    public:
        SceneObject* object = nullptr;

        virtual ~Component() = default;

        virtual void OnStart(){}
        virtual void OnDestroy(){}
        virtual void OnUpdate(f64 deltaTime){}

        static void RegisterType(NativeTypeHandler<Component>& type);
    };
}
