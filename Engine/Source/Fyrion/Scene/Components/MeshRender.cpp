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
        if (activated)
        {
            RenderStorage::RemoveMeshFromRender(reinterpret_cast<usize>(this));
        }
    }

    void MeshRender::OnNotify(i64 type, VoidPtr userData)
    {
        switch (type)
        {
            case SceneNotifications_OnComponentCreated:
            case SceneNotifications_OnActivate:
            {
                transformComponent = object->GetComponent<TransformComponent>();
                activated = true;
                OnChange();
                break;
            }

            case SceneNotifications_OnDeactivate:
            {
                activated = false;
                RenderStorage::RemoveMeshFromRender(reinterpret_cast<usize>(this));
                break;
            }

            case SceneNotifications_OnComponentAdded:
            {
                if (*static_cast<TypeID*>(userData) == GetTypeID<TransformComponent>() && transformComponent == nullptr)
                {
                    transformComponent = object->GetComponent<TransformComponent>();
                    OnChange();
                }
                break;
            }

            case SceneNotifications_OnComponentRemoved:
            {
                if (*static_cast<TypeID*>(userData) == GetTypeID<TransformComponent>() && transformComponent != nullptr)
                {
                    transformComponent = nullptr;
                    OnChange();
                }
                break;
            }

            case SceneNotifications_TransformChanged:
            {
                OnChange();
                break;
            }
            default: break;
        }
    }

    void MeshRender::OnChange()
    {
        if (activated)
        {
            if (transformComponent != nullptr && mesh != nullptr)
            {
                RenderStorage::AddOrUpdateMeshToRender(reinterpret_cast<usize>(this), transformComponent->GetWorldTransform(), mesh);
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

    void MeshRender::RegisterType(NativeTypeHandler<MeshRender>& type)
    {
        type.Field<&MeshRender::mesh>("mesh").Attribute<UIProperty>();
    }
}
