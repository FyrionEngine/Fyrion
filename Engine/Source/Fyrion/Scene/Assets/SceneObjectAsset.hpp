#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Scene/Component.hpp"
#include "Fyrion/Scene/SceneObject.hpp"


namespace Fyrion
{
    class FY_API SceneObjectAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        StringView GetDisplayName() const override
        {
            return "Scene Object";
        }

        SceneObject& GetObject()
        {
            return object;
        }

        void SetName(StringView p_name) override;

        static void RegisterType(NativeTypeHandler<SceneObjectAsset>& type);

    private:
        SceneObject object{this};
    };
}
