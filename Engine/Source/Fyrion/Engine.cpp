#include "Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Platform/PlatformTypes.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include <iostream>

namespace Fyrion
{
    void PlatformInit();
    void PlatformShutdown();

    void RegistryShutdown();

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::Engine");
        bool running = true;
        Window window{};
        f64 lastTime{};
        f64 deltaTime{};
        u64 frame{0};
    }

    void Engine::Init()
    {
        Init(0, nullptr);
    }

    void Engine::Init(i32 argc, char** argv)
    {
        PlatformInit();
    }

    void Engine::CreateContext(const EngineContextCreation& contextCreation)
    {
        WindowFlags windowFlags = WindowFlags::None;
        if (contextCreation.maximize)
        {
            windowFlags |= WindowFlags::Maximized;
        }

        if (contextCreation.fullscreen)
        {
            windowFlags |= WindowFlags::Fullscreen;
        }

        window = Platform::CreateWindow(contextCreation.title, contextCreation.resolution, windowFlags);
    }

    void Engine::Run()
    {
        logger.Info("Fyrion Engine {} Initialized", FY_VERSION);

        while (running)
        {
            f64 currentTime = Platform::GetTime();
            deltaTime = currentTime - lastTime;
            lastTime  = currentTime;

            Platform::ProcessEvents();

            if (Platform::UserRequestedClose(window))
            {
                Engine::Shutdown();
                if (running)
                {
                    Platform::SetWindowShouldClose(window, false);
                }
            }

            frame++;
        }
    }

    void Engine::Shutdown()
    {
        running = false;
    }

    void Engine::Destroy()
    {
        PlatformShutdown();
        RegistryShutdown();
    }
}