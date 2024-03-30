#include "Platform.hpp"
#include "GLFW/glfw3.h"
#include "Fyrion/Core/Logger.hpp"

#ifdef FY_DESKTOP

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::Platform");
    }

    namespace Platform
    {
        void InitStyle();
        void ApplyDarkStyle(VoidPtr internal);
    }


    void PlatformDesktopInit()
    {
        if (!glfwInit())
        {
            logger.Error("Error in initialize glfw");
            return;
        }

        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        Platform::InitStyle();
    }

    void PlatformDesktopShutdown()
    {
        glfwTerminate();
    }


    void Platform::ProcessEvents()
    {
        glfwPollEvents();
    }

    void Platform::WaitEvents()
    {
        glfwWaitEvents();
    }

    Window Platform::CreateWindow(const StringView& title, const Extent& extent, WindowFlags flags)
    {
        bool maximized = (flags && WindowFlags::Maximized);
        glfwWindowHint(GLFW_MAXIMIZED, maximized);

        float xScale = 1.f, yScale = 1.f;
        glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xScale, &yScale);
        Extent size = {u32(extent.width * xScale), u32(extent.height * yScale)};

        GLFWwindow* window = glfwCreateWindow(size.width, size.height, title.CStr(), nullptr, nullptr);

        ApplyDarkStyle(window);

        glfwShowWindow(window);

        return {window};
    }

    Extent Platform::GetWindowExtent(Window window)
    {
        i32 x = 0, y = 0;
        glfwGetWindowSize((GLFWwindow*)window.handler, &x, &y);
        return Extent(x, y);
    }

    bool Platform::UserRequestedClose(Window window)
    {
        return glfwWindowShouldClose((GLFWwindow*)window.handler);
    }

    void Platform::DestroyWindow(Window window)
    {
        glfwDestroyWindow((GLFWwindow*)window.handler);
    }

    f32 Platform::GetWindowScale(Window window)
    {
#ifdef SK_MACOS
        return 1.0f;
#endif

        int xPos, yPos;
        glfwGetWindowPos((GLFWwindow*)window.handler, &xPos, &yPos);

        int xSize, ySize;
        glfwGetWindowSize((GLFWwindow*)window.handler, &xSize, &ySize);

        int monitorCount = 0;
        auto monitors = glfwGetMonitors(&monitorCount);
        for (int i = 0; i < monitorCount; ++i)
        {
            int xMonitorPos, yMonitorPos, monitorWidth, monitorHeight;
            glfwGetMonitorWorkarea(monitors[i], &xMonitorPos, &yMonitorPos, &monitorWidth, &monitorHeight);

            auto x = xPos + xSize / 2;
            auto y = yPos + ySize / 2;

            if (x > xMonitorPos && x < (xMonitorPos + monitorWidth) && y > yMonitorPos && y < (yMonitorPos + monitorHeight))
            {
                float x_scale, y_scale;
                glfwGetMonitorContentScale(monitors[i], &x_scale, &y_scale);
                return x_scale;
            }
        }
        return 1.0f;
    }

    void Platform::SetWindowShouldClose(Window window, bool shouldClose)
    {
        glfwSetWindowShouldClose((GLFWwindow*)window.handler, shouldClose);
    }

    f64 Platform::GetElapsedTime()
    {
        return glfwGetTime();
    }

}

#endif