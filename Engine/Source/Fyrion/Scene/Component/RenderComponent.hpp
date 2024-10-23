#pragma once
#include "Component.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Graphics/Assets/MaterialAsset.hpp"

namespace Fyrion
{
    class MeshAsset;

    class FY_API RenderComponent : public Component
    {
    public:
        FY_BASE_TYPES(Component);

        void SetMesh(MeshAsset* mesh);

        MeshAsset*           GetMesh() const;
        Span<MaterialAsset*> GetMaterials() const;
        void                 OnChange() override;
        void                 OnDestroy() override;

        static void RegisterType(NativeTypeHandler<RenderComponent>& type);

    private:
        MeshAsset*            mesh = nullptr;
        Array<MaterialAsset*> materials = {};
    };
}
