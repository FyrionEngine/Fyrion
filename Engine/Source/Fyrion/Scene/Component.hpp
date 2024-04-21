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

        void SetUpdateEnabled(bool enabled);

        virtual ~Component() = default;

        virtual void OnStart(){}
        virtual void OnDestroy(){}
        virtual void OnUpdate(f64 deltaTime){}

        static void RegisterType(NativeTypeHandler<Component>& type);

        friend class SceneGlobals;
    private:
        u64 m_updateIndex{U64_MAX};
    };
}
