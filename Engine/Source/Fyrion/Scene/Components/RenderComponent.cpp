#include "RenderComponent.hpp"

#include "Fyrion/Scene/SceneObject.hpp"


namespace Fyrion
{
    void RenderComponent::OnStart()
    {
    }

    void RenderComponent::RegisterType(NativeTypeHandler<RenderComponent>& type)
    {
        type.Field<&RenderComponent::m_resourceGraph>("resourceGraph");
    }
}

