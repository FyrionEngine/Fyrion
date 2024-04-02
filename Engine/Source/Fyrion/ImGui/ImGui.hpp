#pragma once

#include "lib/imgui.h"
#include "Fyrion/Common.hpp"
#include "Fyrion/Platform/PlatformTypes.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"


namespace ImGui
{
    void Init(Fyrion::Window window, Fyrion::Swapchain swapchain);
    void BeginFrame(Fyrion::Window window, Fyrion::f64 deltaTime);
    void Render(Fyrion::RenderCommands& renderCommands);
    void ImGuiShutdown();
}