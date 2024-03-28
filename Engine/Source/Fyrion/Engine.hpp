#pragma once

#include "Common.hpp"

namespace Fyrion
{
    struct FY_API Engine
    {
        static void Init();
        static void Destroy();

        static i32  Run(i32 argc, char** argv);
    };
}
