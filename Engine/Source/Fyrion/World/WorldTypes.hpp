#pragma once

namespace Fyrion
{
    using Entity = u64;
    struct Archetype;
    struct ArchetypeChunk;

    constexpr static Entity NullEntity = 0;
    constexpr static u32 MaxComponentsOnChunk = 128;

    struct EntityContainer
    {
        ArchetypeChunk* chunk;
        u32 chunkIndex;
    };
}