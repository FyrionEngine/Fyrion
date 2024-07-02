#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Registry.hpp"


namespace Fyrion
{
    class SceneObject;

    class FY_API Component
    {
    public:
        TypeHandler* typeHandler = nullptr;
        SceneObject* object = nullptr;

        virtual ~Component() = default;

        virtual void OnStart(){}
        virtual void OnChange() {}
        virtual void OnDestroy(){}
        virtual void OnNotify(i64 type) {}

        static void RegisterType(NativeTypeHandler<Component>& type);

    private:
    };
}
