#pragma once

#include "Fyrion/Common.hpp"
#include "WorldTypes.hpp"
#include "Archetype.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/FixedArray.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{

    class FY_API World final
    {
    public:
        FY_FINLINE World(const StringView& name)
        {
            m_name = name;
            m_rootArchetype = CreateArchetype(nullptr, 0);
        }

        FY_FINLINE virtual ~World()
        {

        }

        World(const World&) = delete;
        World& operator=(const World& world) = delete;

        FY_FINLINE StringView  GetName() const { return m_name; }

        FY_FINLINE Entity Spawn()
        {
            return m_entityCounter++;
        }

        FY_FINLINE Entity Spawn(const RID& entityAsset, Entity parent, TypeID* types, VoidPtr* components, usize size)
        {
            Entity entity = m_entityCounter++;
            Add(entity, types, components, size);
        }

        template<typename ...Types>
        FY_FINLINE Entity Spawn(Types&&...types)
        {
            FixedArray<TypeID, sizeof...(Types)> ids{GetTypeID<Types>()...};
            FixedArray<VoidPtr,  sizeof...(Types)> components{&types...};
            return Spawn({}, nullEntity, ids.begin(), components.begin(), sizeof...(Types));
        }

        FY_FINLINE void Add(Entity entity, TypeID* types, VoidPtr* components, usize size)
        {

        }

    private:
        String m_name{};
        std::atomic_uint64_t m_entityCounter{1};
        HashMap<TypeID, Array<UniquePtr<Archetype>>> m_archetypes{};
        Archetype* m_rootArchetype{};

        FY_FINLINE Archetype* FindOrCreateArchetype(usize hash, TypeID* types, usize size)
        {
            return nullptr;
        }

        FY_FINLINE Archetype* CreateArchetype(TypeID* types, usize size)
        {
            return nullptr;
        }
    };

}