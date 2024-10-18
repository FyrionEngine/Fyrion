#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    class GameObject;

    class FY_API Component
    {
    public:
        friend class GameObject;

    private:
        TypeID      typeId = 0;
        GameObject* gameObject = nullptr;
    };
}
