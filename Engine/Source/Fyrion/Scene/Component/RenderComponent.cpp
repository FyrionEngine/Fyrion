#include "RenderComponent.hpp"

#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/Assets/MeshAsset.hpp"

namespace Fyrion
{
    MeshAsset* RenderComponent::GetMesh() const
    {
        return mesh;
    }

    Span<MaterialAsset*> RenderComponent::GetMaterials() const
    {
        return materials;
    }

    void RenderComponent::OnChange()
    {
        if (mesh)
        {
            materials = mesh->materials;
        }
    }

    void RenderComponent::OnDestroy()
    {

    }

    void RenderComponent::SetMesh(MeshAsset* mesh)
    {
        this->mesh = mesh;
        OnChange();
    }

    void RenderComponent::RegisterType(NativeTypeHandler<RenderComponent>& type)
    {
        type.Field<&RenderComponent::mesh>("mesh").Attribute<UIProperty>();
        type.Field<&RenderComponent::materials>("materials").Attribute<UIProperty>().Attribute<UIArrayProperty>(UIArrayProperty{.canAdd = false, .canRemove = false});
    }
}
