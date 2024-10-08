#pragma once

#include "Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Platform/PlatformTypes.hpp"

namespace Fyrion
{
    class RenderCommands;

    using OnInit = EventType<"Fyrion::OnInit"_h, void()>;
    using OnBeginFrame = EventType<"Fyrion::OnBeginFrame"_h, void()>;
    using OnUpdate = EventType<"Fyrion::OnUpdate"_h, void(f64 deltaTime)>;
    using OnEndFrame = EventType<"Fyrion::OnEndFrame"_h, void()>;
    using OnShutdown = EventType<"Fyrion::OnShutdown"_h, void()>;
    using OnShutdownRequest = EventType<"Fyrion::OnShutdownRequest"_h, void(bool* canClose)>;
    using OnRecordRenderCommands = EventType<"Fyrion::OnRecordRenderCommands"_h, void(RenderCommands& renderCommands, f64 deltaTime)>;
    using OnSwapchainRender = EventType<"Fyrion::OnSwapchainRender"_h, void(RenderCommands& renderCommands)>;

    struct EngineContextCreation
    {
        StringView title{};
        Extent     resolution{};
        bool       maximize{false};
        bool       fullscreen{false};
        bool       headless = false;
    };


    struct FY_API Engine
    {
        static void   Init();
        static void   Init(i32 argc, char** argv);
        static void   CreateContext(const EngineContextCreation& contextCreation);
        static void   Run();
        static void   Shutdown();
        static void   Destroy();
        static bool   IsRunning();
        static u64    GetFrame();
        static f64    DeltaTime();
        static Window GetActiveWindow();
        static Extent GetViewportExtent();

        static StringView GetArgByName(const StringView& name);
        static StringView GetArgByIndex(usize i);
        static bool       HasArgByName(const StringView& name);
    };
}
