#pragma once
#include "WorldTypes.hpp"
#include "Fyrion/Core/Array.hpp"

namespace Fyrion
{
    struct ComponentState
    {
        u64 lastChange{};
        u64 lastCheck{};
    };

    struct ArchetypeChunkColumn
    {
        Array<u8>               data{};
        Array<ComponentState>   state{};
    };

    struct ArchetypeChunk
    {
        Archetype* archetype;
        Array<Entity> entities{};
        usize entityCount{};
    };

    struct Archetype
    {
        u64 hashId{};
        ArchetypeChunk* activeChunk{};
    };


}