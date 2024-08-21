#pragma once

#include <iostream>
#include "Fyrion/Platform/Platform.hpp"

namespace Fyrion
{
    struct Chronometer
    {
        f64 startTime{};

        Chronometer() : startTime(Platform::GetTime()) {}

        f64 Get()
        {
            return ((Platform::GetTime() - startTime) * 1000);
        }

        void Reset()
        {
            startTime = Platform::GetTime();
        }

        void Print() const
        {
            std::cout << "time spent: " << ((Platform::GetTime() - startTime) * 1000) << "ms " << std::endl;
        }

        void Print(const char* msg) const
        {
            std::cout << msg << " time spent: " << ((Platform::GetTime() - startTime) * 1000) << "ms " << std::endl;
        }

        f64 Diff() const
        {
            return ((Platform::GetTime() - startTime) * 1000);
        }
    };
}
