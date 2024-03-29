#include "Engine.hpp"
#include "Fyrion/Core/Logger.hpp"
#include <iostream>

namespace Fyrion
{

    void RegistryShutdown();

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::Engine");
    }


    void Engine::Init()
    {

    }

    void Engine::Destroy()
    {
        RegistryShutdown();
    }

    i32 Engine::Run(i32 argc, char** argv)
    {
        logger.Info("Fyrion Engine {} Initialized", FY_VERSION);
        return 0;
    }

}