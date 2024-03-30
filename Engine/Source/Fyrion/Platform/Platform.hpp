#pragma once

#include "PlatformTypes.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Math.hpp"

namespace Fyrion::Platform
{
    FY_API void         ProcessEvents();
    FY_API void         WaitEvents();

    FY_API Window       CreateWindow(const StringView& title, const Extent& extent, WindowFlags flags);
    FY_API Extent       GetWindowExtent(Window window);
    FY_API bool         UserRequestedClose(Window window);
    FY_API void         DestroyWindow(Window window);
    FY_API f32          GetWindowScale(Window window);
    FY_API void         SetWindowShouldClose(Window window, bool shouldClose);


    FY_API f64          GetTime();
    FY_API f64          GetElapsedTime();

}