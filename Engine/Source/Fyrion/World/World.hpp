#pragma once

#include "WorldTypes.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/SharedPtr.hpp"

namespace Fyrion
{
    class FY_API World
    {
    public:
        Entity Spawn()
        {
            return CreateEntity();
        }

        template <typename... T>
        Entity Spawn(T&&... components)
        {
            Entity entity = CreateEntity();
            (new(AssureStorage<T>()->Emplace(entity)) T{Traits::Forward<T>(components)}, ...);
            return entity;
        }


        template <typename T>
        T& Get(Entity entity)
        {
            return *static_cast<T*>(AssureStorage<T>()->Get(entity));
        }

    private:
        Entity CreateEntity()
        {
            return entityCount++;
        }

        template <typename T>
        Storage* AssureStorage()
        {
            return AssureStorage(GetTypeID<T>());
        }

        Storage* AssureStorage(TypeID typeId)
        {
            auto it = pools.Find(typeId);
            if (it == pools.end())
            {
                TypeHandler* typeHandler = Registry::FindTypeById(typeId);
                FY_ASSERT(typeHandler, "type not found");
                it = pools.Emplace(typeId, MakeShared<Storage>(typeHandler)).first;
            }
            return it->second.Get();
        }

        usize entityCount = 1;

        HashMap<TypeID, SharedPtr<Storage>> pools;
    };
}
