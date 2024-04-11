#pragma once
#include "WorldTypes.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Core/Hash.hpp"

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

    struct ArchetypeType
    {
        TypeID typeId{};
        TypeHandler* typeHandler{};
        ComponentSparse* sparse{};
    };

    struct Archetype
    {
        u64 hashId{};
        ArchetypeChunk* activeChunk{};
        Array<ArchetypeType> types{};
        HashMap<TypeID, u16> typeIndex{};
        u32 maxEntityChunkCount{};
    };

    struct ArchetypeLookup
    {
        u64 hashId{};
        Array<TypeID> ids{};

        ArchetypeLookup(u64 hashId, const Span<TypeID>& ids) : hashId(hashId), ids(ids)
        {}

        explicit ArchetypeLookup(Span<TypeID> ids) : ids(ids)
        {
            hashId = Sum(ids.begin(), ids.end());
        }

        inline bool operator==(const ArchetypeLookup& other) const
        {
            return this->ids == other.ids;
        }

        inline bool operator!=(const ArchetypeLookup& other) const
        {
            return !((*this) == other);
        }

        inline bool operator==(Span<TypeID> otherIds) const
        {
            return this->ids == otherIds;
        }
    };


    template<>
    struct Hash<ArchetypeLookup>
    {
        constexpr static bool hasHash = true;

        constexpr static usize Value(const ArchetypeLookup& value)
        {
            return value.hashId;
        }

        constexpr static usize Value(Span<TypeID> ids)
        {
            return Sum(ids.begin(), ids.end());
        }
    };


}