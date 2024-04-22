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

        void EnableComponentUpdate();
        void DisableComponentUpdate();

        virtual ~Component() = default;

        virtual void OnStart(){}
        virtual void OnDestroy(){}
        virtual void OnUpdate(f64 deltaTime){}
        virtual void OnNotify(i64 type) {}

        static void RegisterType(NativeTypeHandler<Component>& type);

        friend class SceneGlobals;
        friend class SceneObject;
    private:
        u64     m_updateIndex{U64_MAX};
        bool    m_started{};
    };
}
