#include "Component.hpp"

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

    void Component::RegisterType(NativeTypeHandler<Component>& type)
    {

    }
}

