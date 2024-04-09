#include "doctest.h"
#include "Fyrion/World/World.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/World/Query.hpp"

using namespace Fyrion;

namespace
{
    struct ComponentOne
    {

    };

    struct ComponentTwo
    {

    };


    TEST_CASE("World::Basics")
    {
        Engine::Init();

        World world("TestWorld");
        CHECK(world.Spawn() == 0);
        CHECK(world.Spawn() == 1);




        world.Spawn();



        Engine::Shutdown();
    }
}