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

    TEST_CASE("World::InternalFunctionTests")
    {
        World world("TestWorld");

        EntityContainer& entityContainer = world.FindOrCreateEntityContainer(50);
        CHECK(entityContainer.chunk == nullptr);
        CHECK(entityContainer.chunkIndex == 0);
    }


    TEST_CASE("World::Basics")
    {
        Engine::Init();

        World world("TestWorld");
        CHECK(world.Spawn() == 1);
        CHECK(world.Spawn() == 2);


        Engine::Shutdown();
    }
}