#pragma once

#include "WorldTypes.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion
{
    class FY_API World
    {
    public:
        void Add(Entity entuty, TypeID* types, ConstPtr* components, i16 size)
        {

        }

        void Remove(Entity entity, TypeID* types, usize size)
        {

        }

        void Destroy(Entity entity)
        {

        }

    private:
        ArchetypeHashMap archetypes;
    };
}
