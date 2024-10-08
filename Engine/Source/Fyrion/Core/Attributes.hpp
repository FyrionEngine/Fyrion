#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    struct UIProperty {};

    struct UIFloatProperty
    {
        f32 minValue{};
        f32 maxValue{};
    };

    struct UIArrayProperty
    {
        bool canAdd = true;
        bool canRemove = true;
    };
}
