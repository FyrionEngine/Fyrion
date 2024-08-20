#pragma once
#include "Fyrion/Common.hpp"


namespace Fyrion
{
    using Entity = u64;

    struct ComponentState
    {
        u64 lastChange = 0;
        u64 lastCheck = 0;
    };

    struct EntityData
    {
        bool alive = true;
    };

    namespace Internal
    {
        constexpr static u16 ChunkSize = 1024;
        constexpr static u32 CountOffset = 0;
        constexpr static u32 EntitiesOffset = CountOffset + sizeof(usize);
        constexpr static u32 CompStatesOffset = EntitiesOffset + ChunkSize * sizeof(Entity);
        constexpr static u32 ChunkStateOffset = CompStatesOffset + ChunkSize * sizeof(ComponentState);
        constexpr static u32 DataOffset = ChunkStateOffset + sizeof(ComponentState);
    }
}
