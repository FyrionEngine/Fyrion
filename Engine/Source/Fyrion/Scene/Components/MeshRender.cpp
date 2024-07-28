#include "MeshRender.hpp"

#include <Fyrion/Scene/SceneTypes.hpp>

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Scene/SceneObject.hpp"
#include "Fyrion/Graphics/Assets/MeshAsset.hpp"

#include "TransformComponent.hpp"
#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Graphics/RenderStorage.hpp"


namespace Fyrion
{
    MeshRender::~MeshRender()
    {
        if (object->IsActivated())
        {
            RenderStorage::RemoveMeshFromRender(reinterpret_cast<usize>(this));
        }
    }

    void MeshRender::OnNotify(const NotificationEvent& notificationEvent)
    {
        switch (notificationEvent.type)
        {
            case SceneNotifications_OnActivated:
            {
                if (!transformComponent)
                {
                    transformComponent = object->GetComponent<TransformComponent>();
                }
                if (transformComponent && mesh)
                {
                    OnChange();
                }
                break;
            }

            case SceneNotifications_OnDeactivated:
            {
                RenderStorage::RemoveMeshFromRender(reinterpret_cast<usize>(this));
                break;
            }

            case SceneNotifications_OnComponentRemoved:
            {
                if (transformComponent == notificationEvent.component)
                {
                    transformComponent = nullptr;
                    OnChange();
                }
            }

            case SceneNotifications_TransformChanged:
            {
                transformComponent = static_cast<TransformComponent*>(notificationEvent.component);
                if (transformComponent && mesh)
                {
                    OnChange();
                }
                break;
            }
        }
    }

    void MeshRender::OnChange()
    {
        if (object->IsActivated())
        {
            if (materials.Empty() && mesh != nullptr)
            {
                materials = mesh->GetMaterials();
            }
            else if (activeMesh != nullptr && activeMesh != mesh)
            {
                materials.Clear();
                if (mesh != nullptr)
                {
                    materials = mesh->GetMaterials();
                }
            }

            activeMesh = mesh;

            if (transformComponent != nullptr && mesh != nullptr)
            {
                RenderStorage::AddOrUpdateMeshToRender(reinterpret_cast<usize>(this), transformComponent->GetWorldTransform(), mesh, materials);
            }
            else
            {
                RenderStorage::RemoveMeshFromRender(reinterpret_cast<usize>(this));
            }
        }
    }

    void MeshRender::SetMesh(MeshAsset* p_mesh)
    {
        mesh = p_mesh;
        OnChange();
    }

    MeshAsset* MeshRender::GetMesh() const
    {
        return mesh;
    }

    Span<MaterialAsset*> MeshRender::GetMaterials() const
    {
        return materials;
    }

    void MeshRender::RegisterType(NativeTypeHandler<MeshRender>& type)
    {
        type.Field<&MeshRender::mesh>("mesh").Attribute<UIProperty>();
        type.Field<&MeshRender::materials>("materials").Attribute<UIProperty>().Attribute<UIArrayProperty>(UIArrayProperty{.canAdd = false, .canRemove = false});
    }
}
