#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    class GameObject;

    class FY_API Component : public Object
    {
    public:
        friend class GameObject;

        virtual void OnStart() {}
        virtual void OnNotify(u64 type) {}
        virtual void OnDestroy() {}
        virtual void OnChange() {}

        GameObject* gameObject = nullptr;
        TypeID      typeId = 0;

        static void RegisterType(NativeTypeHandler<Component>& type);
    };
}
