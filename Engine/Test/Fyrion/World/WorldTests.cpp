#include "doctest.h"
#include "Fyrion/World/World.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/World/Query.hpp"

using namespace Fyrion;

namespace
{
    constexpr const char* HUGE = "HUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGEHUGE";


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

    struct ComponentThree
    {
        i32 otherValue{};
    };

    void RegisterTypes()
    {
        Registry::Type<ComponentOne>();
        Registry::Type<ComponentTwo>();
        Registry::Type<ComponentThree>();
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

        Engine::Destroy();
    }

    struct Imcomplete;

    TEST_CASE("World::Basics")
    {
        CHECK(Traits::IsTriviallyCopyable<ComponentOne>);
        CHECK(!Traits::IsTriviallyCopyable<ComponentTwo>);
        CHECK(!Traits::IsTriviallyCopyable<Imcomplete>);

        Engine::Init();
        RegisterTypes();
        {
            World world("TestWorld");
            CHECK(world.Spawn() == 1);
            CHECK(world.Spawn() == 2);

            Entity entity1 = world.Spawn(
                ComponentOne{.intValue = 10},
                ComponentTwo{.strValue = HUGE}
            );

            CHECK(entity1 > 0);
            {
                EntityContainer& container = world.FindOrCreateEntityContainer(entity1);
                CHECK(container.archetype != nullptr);
                CHECK(container.chunk != nullptr);
                CHECK(container.chunkIndex == 0);

                CHECK(world.Get<ComponentOne>(entity1).intValue == 10);
                CHECK(world.Get<ComponentTwo>(entity1).strValue == HUGE);

                CHECK(FY_CHUNK_ENTITY(container.archetype, container.chunk, container.chunkIndex) == entity1);
            }

            Entity entity2 = world.Spawn(
                ComponentOne{.intValue = 40},
                ComponentTwo{.strValue = "Asd"}
            );

            CHECK(entity2 > 0);
            {
                EntityContainer& container = world.FindOrCreateEntityContainer(entity2);
                CHECK(container.archetype != nullptr);
                CHECK(container.chunk != nullptr);
                CHECK(container.chunkIndex == 1);

                CHECK(world.Get<ComponentOne>(entity2).intValue == 40);
                CHECK(world.Get<ComponentTwo>(entity2).strValue == "Asd");

                CHECK(FY_CHUNK_ENTITY(container.archetype, container.chunk, container.chunkIndex) == entity2);
            }

            CHECK(world.Get<ComponentOne>(entity1).intValue == 10);
            CHECK(world.Get<ComponentTwo>(entity1).strValue == HUGE);

            world.Add(entity1, ComponentThree{.otherValue = 444});

            CHECK(world.Get<ComponentOne>(entity1).intValue == 10);
            CHECK(world.Get<ComponentTwo>(entity1).strValue == HUGE);
            CHECK(world.Get<ComponentThree>(entity1).otherValue == 444);

            {
                EntityContainer& container1 = world.FindOrCreateEntityContainer(entity1);
                EntityContainer& container2 = world.FindOrCreateEntityContainer(entity2);
                CHECK(container1.archetype != container2.archetype);

                Archetype* archetype = world.FindArchetype({GetTypeID<ComponentTwo>(), GetTypeID<ComponentOne>()});
                CHECK(archetype->chunks.Size() == 1);
            }

            world.Add(entity2, ComponentThree{.otherValue = 555});

            CHECK(world.Get<ComponentOne>(entity2).intValue == 40);
            CHECK(world.Get<ComponentTwo>(entity2).strValue == "Asd");
            CHECK(world.Get<ComponentThree>(entity2).otherValue == 555);

            {
                EntityContainer& container1 = world.FindOrCreateEntityContainer(entity1);
                EntityContainer& container2 = world.FindOrCreateEntityContainer(entity2);
                CHECK(container1.archetype == container2.archetype);
                CHECK(container1.chunk == container2.chunk);
                CHECK(container1.chunkIndex != container2.chunkIndex);

                Archetype* archetype = world.FindArchetype({GetTypeID<ComponentTwo>(), GetTypeID<ComponentOne>()});
                CHECK(archetype->chunks.Size() == 0);
            }

            CHECK(!world.Remove<Imcomplete>(entity1));
            CHECK(world.Remove<ComponentOne>(entity1));

            CHECK(!world.Has<ComponentOne>(entity1));
            CHECK(world.Get<ComponentTwo>(entity1).strValue == HUGE);
            CHECK(world.Get<ComponentThree>(entity1).otherValue == 444);

        }
        Engine::Destroy();
    }
}
