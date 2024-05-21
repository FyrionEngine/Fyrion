#include "RenderComponent.hpp"

#include "Fyrion/Graphics/DefaultRenderPipeline/GraphNodes.hpp"
#include "Fyrion/Resource/ResourceGraph.hpp"
#include "Fyrion/Scene/SceneObject.hpp"


namespace Fyrion
{
    void RenderComponent::OnStart()
    {
    }

    void RenderComponent::RegisterType(NativeTypeHandler<RenderComponent>& type)
    {
        type.Field<&RenderComponent::m_resourceGraph>("resourceGraph").Attribute<ResourceReference>(ResourceReference{
            .resourceType = GetTypeID<ResourceGraphAsset>(),
            .graphOutput = GetTypeID<GeometryRender>()
        });
    }
}

