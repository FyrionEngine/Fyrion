#pragma once
#include "Fyrion/Scene/Component.hpp"

namespace Fyrion
{
    class MeshAsset;
    class TransformComponent;

    class MeshRender : public Component
    {
    public:
        FY_BASE_TYPES(Component);

        void OnNotify(i64 type, VoidPtr userData) override;
        void OnChange() override;

        void       SetMesh(MeshAsset* p_mesh);
        MeshAsset* GetMesh() const;

        static void RegisterType(NativeTypeHandler<MeshRender>& type);

    private:
        MeshAsset*          mesh = nullptr;
        TransformComponent* transformComponent = nullptr;
        bool                activated = false;
    };
}
