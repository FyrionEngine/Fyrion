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
            CHECK(archetypeOneComp->chunkAllocSize > 0);

            Archetype* archetypeTwoComps = world.CreateArchetype({GetTypeID<ComponentOne>(), GetTypeID<ComponentTwo>()});
            CHECK(archetypeTwoComps != nullptr);
            CHECK(archetypeOneComp != archetypeTwoComps);

            CHECK(archetypeTwoComps->maxEntityChunkCount > 0);
            CHECK(archetypeTwoComps->chunkAllocSize > 0);

            Archetype* archetypeSorted = world.CreateArchetype({GetTypeID<ComponentTwo>(), GetTypeID<ComponentOne>()});
            CHECK(archetypeTwoComps == archetypeSorted);
        }

        {
            World world("TestWorld");
            Archetype* archetype = world.CreateArchetype({GetTypeID<ComponentOne>()});
            CharPtr chunkData = world.FindOrCreateChunk(archetype);
            CHECK(chunkData != nullptr);
        }

        Engine::Shutdown();
    }

    struct Imcomplete;

    TEST_CASE("World::Basics")
    {

        CHECK(Traits::IsTriviallyCopyable<ComponentOne>);
        CHECK(!Traits::IsTriviallyCopyable<ComponentTwo>);
        CHECK(!Traits::IsTriviallyCopyable<Imcomplete>);

        Engine::Init();

        World world("TestWorld");
        CHECK(world.Spawn() == 1);
        CHECK(world.Spawn() == 2);

        Entity entity1 = world.Spawn(
            ComponentOne{.intValue = 10},
            ComponentTwo{.strValue = "Asd"}
        );
        CHECK(entity1 > 0);
        {
            EntityContainer& container = world.FindOrCreateEntityContainer(entity1);
            CHECK(container.archetype != nullptr);
            CHECK(container.chunk != nullptr);
            CHECK(container.chunkIndex == 0);
        }

        Entity entity2 = world.Spawn(
            ComponentOne{.intValue = 10},
            ComponentTwo{.strValue = "Asd"}
        );
        CHECK(entity2 > 0);

        {
            EntityContainer& container = world.FindOrCreateEntityContainer(entity2);
            CHECK(container.archetype != nullptr);
            CHECK(container.chunk != nullptr);
            CHECK(container.chunkIndex == 1);
        }

        Engine::Shutdown();
    }
}