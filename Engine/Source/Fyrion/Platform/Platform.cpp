#include "Platform.hpp"

namespace Fyrion
{
    void PlatformDesktopInit();
    void PlatformDesktopShutdown();

    void PlatformInit()
    {
        PlatformDesktopInit();
    }

    void PlatformShutdown()
    {
        PlatformDesktopShutdown();
    }
}