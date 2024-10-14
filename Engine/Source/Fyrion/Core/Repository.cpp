#include "Repository.hpp"

#include "HashMap.hpp"
#include "Registry.hpp"

namespace Fyrion
{
    namespace
    {
        struct Storage
        {
            TypeHandler*                 typeHandler;
            VoidPtr                      instance;
            VoidPtr                      loaderData;
            FnResourceLoader loader;
        };

        HashMap<UUID, Storage> resources;
    }

    void Repository::Register(UUID uuid, TypeID typeId, bool instantiate, VoidPtr loaderData, FnResourceLoader loader)
    {
        TypeHandler* typeHandler = Registry::FindTypeById(typeId);
        VoidPtr      instance = nullptr;

        if (instantiate)
        {
            instance = typeHandler->NewInstance();
        }

        resources.Emplace(uuid, Storage{
                              .typeHandler = typeHandler,
                              .instance = instance,
                              .loaderData = loaderData,
                              .loader = loader
                          });
    }

    VoidPtr Repository::Load(UUID uuid)
    {
        if (auto it = resources.Find(uuid))
        {
            if (!it->second.instance)
            {
                it->second.instance = it->second.typeHandler->NewInstance();

                if (it->second.loader)
                {
                    it->second.loader(it->second.loaderData, it->second.instance, it->second.typeHandler);
                }
            }
            return it->second.instance;
        }
        return nullptr;
    }

    TypeHandler* Repository::GetTypeHandler(UUID uuid)
    {
        if (auto it = resources.Find(uuid))
        {
            return it->second.typeHandler;
        }
        return nullptr;
    }

    void RepositoryShutdown()
    {
        for(auto it: resources)
        {
            if (it.second.instance && it.second.typeHandler)
            {
                it.second.typeHandler->Destroy(it.second.instance);
            }
        }
        resources.Clear();
    }

    void RepositoryInit()
    {
        RepositoryShutdown();
    }


}
