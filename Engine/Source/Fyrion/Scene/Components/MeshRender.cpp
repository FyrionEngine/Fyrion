#include "MeshRender.hpp"

#include "Fyrion/Core/Registry.hpp"


namespace Fyrion
{
    void MeshRender::RegisterType(NativeTypeHandler<MeshRender>& type)
    {
        type.Field<&MeshRender::mesh>("mesh");
    }
}
