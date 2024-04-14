#pragma once

#include "Fyrion/Core/Sparse.hpp"

namespace Fyrion
{
    using Entity = u64;
    struct Archetype;
    class World;

    typedef Sparse<VoidPtr, nullptr> ComponentSparse;

    constexpr static Entity NullEntity = 0;
    constexpr static u32 MaxComponentsOnChunk = 128;

    struct EntityContainer
    {
        Archetype*  archetype{};
        CharPtr     chunk{};
        u32         chunkIndex{};
    };
}