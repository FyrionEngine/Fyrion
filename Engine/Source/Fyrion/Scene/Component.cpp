#include "Component.hpp"

#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void Component::SetUUID(const UUID& uuid)
    {
        this->uuid = uuid;
    }

    const UUID& Component::GetUUID() const
    {
        return uuid;
    }

    void Component::SetPrototype(const UUID& prototype)
    {
        this->prototype = prototype;
    }

    UUID Component::GetPrototype() const
    {
        return prototype;
    }

    void Component::RegisterType(NativeTypeHandler<Component>& type)
    {
    }
}

