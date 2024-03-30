#pragma once

#include "Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Math.hpp"

namespace Fyrion
{
    struct EngineContextCreation
    {
        StringView  title{};
        Extent      resolution{};
        bool        maximize{false};
        bool        fullscreen{false};
        bool        headless = false;
    };


    struct FY_API Engine
    {
        static void Init();
        static void Init(i32 argc, char** argv);
        static void CreateContext(const EngineContextCreation& contextCreation);
        static void Run();
        static void Shutdown();
        static void Destroy();
    };
}
