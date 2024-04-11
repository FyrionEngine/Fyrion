#pragma once
#include "WorldTypes.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Core/Hash.hpp"

namespace Fyrion
{
#define FY_CHUNK_ENTITY_COUNT(archetype, chunk) reinterpret_cast<u32&>(chunk[archetype->entityCountOffset])
#define FY_CHUNK_ENTITY_SET(archetype, chunk, index, entity) reinterpret_cast<Entity&>(chunk[archetype->entityArrayOffset + (index * sizeof(Entity))]) = entity
#define FY_CHUNK_ENTITY_GET(archetype, chunk, index) reinterpret_cast<Entity&>(chunk[archetype->entityArrayOffset + (index * sizeof(Entity))])
#define FY_CHUNK_COMPONENT_DATA(type, chunk, index) &chunk[type.dataOffset + (index * type.typeHandler->GetTypeInfo().size)]
#define FY_CHUNK_COMPONENT_STATE(type, chunk, index) *reinterpret_cast<ComponentState*>(&chunk[type.stateOffset + (index * sizeof(ComponentState))])
#define FY_CHUNK_STATE(typeIndex, archetype, chunk) *reinterpret_cast<ComponentState*>(&chunk[archetype->chunkStateOffset + (typeIndex * sizeof(ComponentState))])

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
        bool isTriviallyCopyable{};
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
        u32 chunkDataSize{};
        u32 entityArrayOffset{};
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