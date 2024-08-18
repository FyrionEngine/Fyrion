#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"

namespace Fyrion
{
    using Entity = u64;

    struct ComponentState
    {
        u64 lastChange = 0;
        u64 lastCheck = 0;
    };

    struct ArchetypeType
    {
        TypeID       typeId;
        TypeHandler* typeHandler;
        u16          typeSize;
        usize        dataOffset;
        usize        stateOffset;
    };

    using ArchetypeChunk = u8*;

    struct Archetype
    {
        u32                   id;
        u64                   hash;
        usize                 maxEntityChunkCount;
        usize                 chunkTotalAllocSize;
        usize                 chunkDataSize;
        usize                 entityArrayOffset;
        usize                 entityCountOffset;
        usize                 chunkStateOffset;
        Array<ArchetypeType>  types;
        HashMap<TypeID, u16>  typeIndex;
        Array<ArchetypeChunk> chunks;

        usize GetEntityCount(const ArchetypeChunk& chunk) const
        {
            return reinterpret_cast<usize>(&chunk[entityCountOffset]);
        }
    };

    using ArchetypeHashMap = HashMap<usize, Array<Archetype>>;
}
