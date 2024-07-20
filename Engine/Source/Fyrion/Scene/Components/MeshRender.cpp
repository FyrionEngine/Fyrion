#include "MeshRender.hpp"

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Graphics/Assets/MeshAsset.hpp"
#include "Fyrion/Scene/SceneObject.hpp"


namespace Fyrion
{
    void MeshRender::OnChange()
    {
        //object->globals->renderGraph->items->AddMesh();
    }

    void MeshRender::RegisterType(NativeTypeHandler<MeshRender>& type)
    {
        type.Field<&MeshRender::mesh>("mesh");
    }
}
