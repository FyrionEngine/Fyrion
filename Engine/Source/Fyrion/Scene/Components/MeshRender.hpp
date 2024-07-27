#pragma once
#include "Fyrion/Graphics/Assets/MaterialAsset.hpp"
#include "Fyrion/Scene/Component.hpp"

namespace Fyrion
{
    class MeshAsset;
    class TransformComponent;

    class MeshRender : public Component
    {
    public:
        FY_BASE_TYPES(Component);

        ~MeshRender() override;

        void OnNotify(const NotificationEvent& notificationEvent) override;
        void OnChange() override;

        void                 SetMesh(MeshAsset* p_mesh);
        MeshAsset*           GetMesh() const;
        Span<MaterialAsset*> GetMaterials() const;

        static void RegisterType(NativeTypeHandler<MeshRender>& type);

    private:
        MeshAsset*            mesh = nullptr;
        TransformComponent*   transformComponent = nullptr;
        Array<MaterialAsset*> materials = {};
    };
}
