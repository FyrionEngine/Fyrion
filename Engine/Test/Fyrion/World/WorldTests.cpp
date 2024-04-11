#include "doctest.h"
#include "Fyrion/World/World.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/World/Query.hpp"

using namespace Fyrion;

namespace
{
    struct ComponentOne
    {
        i32 intValue{};

        static void RegisterType(NativeTypeHandler<ComponentOne>& type)
        {
            type.Field<&ComponentOne::intValue>("intValue");
        }
    };

    struct ComponentTwo
    {
        String strValue{};

        static void RegisterType(NativeTypeHandler<ComponentOne>& type)
        {
            type.Field<&ComponentTwo::strValue>("strValue");
        }
    };

    void RegisterTypes()
    {
        Registry::Type<ComponentOne>();
        Registry::Type<ComponentTwo>();
    }

    TEST_CASE("World::InternalFunctionTests")
    {
        Engine::Init();
        RegisterTypes();

        {
            World world("TestWorld");
            EntityContainer& entityContainer = world.FindOrCreateEntityContainer(50);
            CHECK(entityContainer.chunk == nullptr);
            CHECK(entityContainer.chunkIndex == 0);
        }

        {
            World world("TestWorld");

            Archetype* archetypeOneComp = world.CreateArchetype({GetTypeID<ComponentOne>()});
            CHECK(archetypeOneComp->maxEntityChunkCount > 0);


            Archetype* archetypeTwoComps = world.CreateArchetype({GetTypeID<ComponentOne>(), GetTypeID<ComponentTwo>()});
            CHECK(archetypeTwoComps != nullptr);
            CHECK(archetypeOneComp != archetypeTwoComps);

            CHECK(archetypeTwoComps->maxEntityChunkCount > 0);

            Archetype* archetypeSorted = world.CreateArchetype({GetTypeID<ComponentTwo>(), GetTypeID<ComponentOne>()});
            CHECK(archetypeTwoComps == archetypeSorted);



        }



        Engine::Shutdown();
    }


    TEST_CASE("World::Basics")
    {
        CHECK(std::is_trivially_copyable_v<ComponentOne>);
        CHECK(!std::is_trivially_copyable_v<ComponentTwo>);

        Engine::Init();

        World world("TestWorld");
        CHECK(world.Spawn() == 1);
        CHECK(world.Spawn() == 2);

        Entity entity = world.Spawn(
            ComponentOne{.intValue = 10},
            ComponentTwo{.strValue = "Asd"}
        );
        CHECK(entity > 0);
        Engine::Shutdown();
    }
}