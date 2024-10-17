#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion
{
    class Scene;

    class FY_API GameObject
    {
    public:
        friend class Scene;

        ~GameObject();

        Scene*      GetScene() const;
        GameObject* GetParent() const;
        StringView  GetName() const;
        void        SetName(StringView newName);
        UUID        GetUUID() const;

        GameObject*       GetPrototype() const;
        Span<GameObject*> GetChildren() const;

        GameObject* CreateChild();
        GameObject* CreateChildWithUUID(UUID uuid);
        void        RemoveChild(GameObject* gameObject);

        void Destroy();

        friend class Scene;
    private:
        GameObject(Scene* scene);
        GameObject(Scene* scene, GameObject* parent);

        Scene*      scene;
        GameObject* parent;
        String      name;
        UUID        uuid;

        Array<GameObject*> children;
    };
}
