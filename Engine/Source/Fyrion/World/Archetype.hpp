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
        u8*                 data{};
        ComponentState*     state{};
    };

    struct ArchetypeChunk
    {
        Archetype*              archetype{};
        Entity*                 entities{};
        ArchetypeChunkColumn*   columns{};
        ComponentState*         chunkStates{};
        usize                   entityCount{};
    };

    struct Archetype
    {
        u64 hashId{};
        ArchetypeChunk* activeChunk{};
    };


}