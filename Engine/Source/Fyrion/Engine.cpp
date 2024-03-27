#include "Engine.hpp"
#include <iostream>

namespace Fyrion
{
    i32 Engine::Run(i32 argc, char** argv)
    {
        std::cout << "Hello, Fyrion" << std::endl;
        return 0;
    }
}