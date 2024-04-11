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

    struct ArchetypeType
    {
        TypeID typeId{};
        TypeHandler* typeHandler{};
        ComponentSparse* sparse{};
        u32 dataOffset{};
        u32 stateOffset{};
    };

    struct Archetype
    {
        u32 index{};
        u64 hashId{};
        CharPtr activeChunk{};
        Array<ArchetypeType> types{};
        HashMap<TypeID, u16> typeIndex{};
        u32 maxEntityChunkCount{};
        u32 chunkAllocSize{};
        u32 entityCountOffset{};
        u32 chunkStateOffset{};
        Array<CharPtr> chunks{};
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