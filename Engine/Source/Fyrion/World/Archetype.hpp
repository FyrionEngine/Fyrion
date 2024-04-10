#pragma once
#include "WorldTypes.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UniquePtr.hpp"

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
        Array<TypeHandler*> types{};
        HashMap<TypeID, usize> typeIndex{};
        Array<TypeID> typesIds{};
        Array<UniquePtr<ArchetypeChunk>> chunks{};
        Array<usize> typesSize{};
        Array<usize> columnAllocationSize{};
        usize stride{};
        usize maxEntityChunkCount{};
        Array<ComponentSparse*> sparses{};
    };


}