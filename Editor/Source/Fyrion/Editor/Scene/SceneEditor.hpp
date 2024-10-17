#pragma once
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Scene/Scene.hpp"


namespace Fyrion {
    class Component;
}

namespace Fyrion
{
    struct AssetFile;

    class SceneEditor
    {
    public:
        Scene*     GetScene() const;
        AssetFile* GetAssetFile() const;
        void       SetScene(AssetFile* assetFile);
        void       ClearSelection();
        void       SelectObject(GameObject& object);
        void       DeselectObject(GameObject& object);
        bool       IsSelected(GameObject& object) const;
        bool       IsParentOfSelected(GameObject& object) const;
        void       RenameObject(GameObject& object, StringView newName);
        void       DestroySelectedObjects();
        void       CreateGameObject();
        bool       IsValidSelection();
        void       AddComponent(GameObject* gameObject, TypeHandler* typeHandler);
        void       ResetComponent(GameObject* gameObject, Component* component);
        void       RemoveComponent(GameObject* gameObject, Component* component);

        void DoUpdate();

    private:
        AssetFile*           assetFile = nullptr;
        Scene*               scene = nullptr;
        HashSet<GameObject*> selectedObjects{};

        EventHandler<OnGameObjectSelection> onGameObjectSelectionHandler{};
    };
}
