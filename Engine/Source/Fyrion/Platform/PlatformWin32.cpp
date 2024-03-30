#include "Platform.hpp"

#ifdef FY_WIN

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Fyrion
{
    static f64              clockFrequency{};
    static LARGE_INTEGER    startTime{};

    void ClockSetup()
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        clockFrequency = 1.0 / (f64)frequency.QuadPart;
        QueryPerformanceCounter(&startTime);
    }

    f64 Platform::GetTime()
    {
        if (clockFrequency == 0)
        {
            ClockSetup();
        }

        LARGE_INTEGER nowTime;
        QueryPerformanceCounter(&nowTime);
        return (f64) nowTime.QuadPart * clockFrequency;
    }
}

#endif