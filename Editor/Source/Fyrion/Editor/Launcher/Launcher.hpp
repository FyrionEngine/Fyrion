#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/String.hpp"


namespace Fyrion::Launcher
{
    FY_API void   Init();
    FY_API void   Shutdown();
    FY_API String GetProject();
}
