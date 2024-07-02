#include "Platform.hpp"
#include <chrono>

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

    f64 Platform::GetTime()
    {
        const std::chrono::time_point<std::chrono::high_resolution_clock> time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::duration<f64>>(time.time_since_epoch()).count();
    }
}