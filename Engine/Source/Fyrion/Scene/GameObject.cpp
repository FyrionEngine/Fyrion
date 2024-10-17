#include "GameObject.hpp"
#include "Scene.hpp"

namespace Fyrion
{
    GameObject::GameObject(Scene* scene) : scene(scene), parent(nullptr) {}
    GameObject::GameObject(Scene* scene, GameObject* parent) : scene(scene), parent(parent) {}


    Scene* GameObject::GetScene() const
    {
        return scene;
    }

    GameObject* GameObject::GetParent() const
    {
        return parent;
    }

    StringView GameObject::GetName() const
    {
        if (parent == nullptr)
        {
            return scene->GetName();
        }
        return name;
    }

    GameObject* GameObject::GetPrototype() const
    {
        return nullptr;
    }

    Span<GameObject*> GameObject::GetChildren() const
    {
        return {};
    }
}
