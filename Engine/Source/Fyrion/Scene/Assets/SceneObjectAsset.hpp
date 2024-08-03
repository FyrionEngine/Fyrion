#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Graphics/Assets/MaterialAsset.hpp"
#include "Fyrion/Scene/Component.hpp"
#include "Fyrion/Scene/SceneObject.hpp"
#include "Fyrion/Scene/SceneTypes.hpp"


namespace Fyrion
{
    class RenderGraphAsset;

    class FY_API SceneObjectAsset : public Asset, public SceneObjectAssetProvider
    {
    public:
        FY_BASE_TYPES(Asset, SceneObjectAssetProvider);

        StringView GetDisplayName() const override
        {
            return "Scene Object";
        }

        SceneObject* GetObject();

        StringView GetDataExtesion() override
        {
            return FY_SCENE_EXTENSION;
        }

        void DestroySceneObject();

        void          DeserializeData(ArchiveReader& reader, ArchiveObject object) override;
        ArchiveObject SerializeData(ArchiveWriter& writer) const override;

        bool LoadData() override;
        void SaveData() override;

        SceneObjectAsset* GetSceneObjectAsset() override;

        static void RegisterType(NativeTypeHandler<SceneObjectAsset>& type);

    private:
        SharedPtr<SceneObject> object{};
        bool                   pendingDestroy = false;
    };
}
