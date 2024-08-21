#pragma once

#include "Storage.hpp"
#include "Query.hpp"
#include "WorldTypes.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/SharedPtr.hpp"

namespace Fyrion::World1
{

    class FY_API World
    {
    public:
        Entity Spawn()
        {
            return entities.CreateEntity();
        }

        template <typename... T>
        Entity Spawn(T&&... components)
        {
            Entity entity = entities.CreateEntity();
            (new(GetStorage<T>()->Emplace(entity, false)) T{Traits::Forward<T>(components)}, ...);
            return entity;
        }

        template <typename... T>
        void Add(Entity entity, T&&... components)
        {
            (new(GetStorage<T>()->Emplace(entity, false)) T{Traits::Forward<T>(components)}, ...);
        }

        template <typename T>
        const T& Get(Entity entity)
        {
            return *static_cast<const T*>(GetStorage<T>()->Get(entity));
        }

        template <typename T>
        T& GetMut(Entity entity)
        {
            return *static_cast<T*>(GetStorage<T>()->Get(entity));
        }

        ConstPtr GetPtr(Entity entity, TypeID typeId)
        {
            return GetStorage(typeId)->Get(entity);
        }

        VoidPtr GetMutPtr(Entity entity, TypeID typeId)
        {
            return GetStorage(typeId)->GetMut(entity);
        }

        template <typename T>
        bool Has(Entity entity) const
        {
            return Has(entity, GetTypeID<T>());
        }

        bool Has(Entity entity, TypeID typeId) const
        {
            if (ComponentStorage* storage = TryGetStorage(typeId))
            {
                return storage->Has(entity);
            }
            return false;
        }

        template <typename T>
        void Remove(Entity entity)
        {
            GetStorage<T>()->Remove(entity);
        }

        void Remove(Entity entity, TypeID typeId)
        {
            GetStorage(typeId)->Remove(entity);
        }

        void Destroy(Entity entity)
        {
            for (auto it : storages)
            {
                if (it.second->Has(entity))
                {
                    it.second->Remove(entity);
                }
            }
            entities.Destroy(entity);
        }

        bool Alive(Entity entity) const
        {
            if (const EntityData* data = entities.GetSafe(entity))
            {
                return data->alive;
            }
            return false;
        }

        usize Count() const
        {
            return entities.Count();
        }

        template <typename T>
        ComponentStorage* GetStorage()
        {
            return GetStorage(GetTypeID<T>());
        }

        ComponentStorage* GetStorage(TypeID typeId)
        {
            auto it = storages.Find(typeId);
            if (it == storages.end())
            {
                TypeHandler* typeHandler = Registry::FindTypeById(typeId);
                FY_ASSERT(typeHandler, "type not found");
                it = storages.Emplace(typeId, MakeShared<ComponentStorage>(typeHandler)).first;
            }
            return it->second.Get();
        }

        ComponentStorage* TryGetStorage(TypeID typeId) const
        {
            if (auto it = storages.Find(typeId))
            {
                return it->second.Get();
            }
            return nullptr;
        }

        template <typename... T>
        decltype(auto) Query()
        {
            return QueryImpl<T...>{
                .storages = {GetStorage<T>()...}
            };
        }

    private:
        EntityStorage                                entities;
        HashMap<TypeID, SharedPtr<ComponentStorage>> storages;
    };
}
