#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Span.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion
{
    class Scene;

    class FY_API GameObject
    {
    public:
        friend class Scene;

        Scene*      GetScene() const;
        GameObject* GetParent() const;
        StringView  GetName() const;

        GameObject*       GetPrototype() const;
        Span<GameObject*> GetChildren() const;

    private:
        GameObject(Scene* scene);
        GameObject(Scene* scene, GameObject* parent);

        Scene*      scene;
        GameObject* parent;
        String      name;
    };
}
