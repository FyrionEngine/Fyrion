#include "SceneObject.hpp"

#include "Component.hpp"
#include "Component.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    SceneObject::SceneObject(StringView name, SceneObject* parent, RID asset) : m_name(name), m_parent(parent), m_asset(asset)
    {
    }

    SceneObject::~SceneObject()
    {
        for (auto it : m_components)
        {
            for (Component* instance : it.second.instances)
            {
                instance->OnDestroy();
                it.second.typeHandler->Destroy(instance);
            }
        }

        for (SceneObject* sceneObject : m_children)
        {
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(sceneObject);
        }
    }

    SceneObject* SceneObject::NewChild(const StringView& name)
    {
        return NewChild(RID{}, name);
    }

    SceneObject* SceneObject::NewChild(const RID& asset, const StringView& name)
    {
        SceneObject* sceneObject = MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>(name, this, asset);
        sceneObject->m_parentIndex = m_children.Size();
        m_children.EmplaceBack(sceneObject);
        return sceneObject;
    }

    SceneObject* SceneObject::GetScene()
    {
        if (m_parent)
        {
            return m_parent->GetScene();
        }
        return this;
    }

    Component& SceneObject::AddComponent(TypeID typeId)
    {
        auto it = m_components.Find(typeId);
        if (it == m_components.end())
        {
            TypeHandler* typeHandler = Registry::FindTypeById(typeId);
            FY_ASSERT(typeHandler, "Type not registered");
            it = m_components.Emplace(typeId, ComponentStorage{.typeHandler = typeHandler}).first;
        }

        ComponentStorage& storage = it->second;
        Component* component = storage.typeHandler->Cast<Component>(storage.typeHandler->NewInstance());
        component->object = this;
        storage.instances.EmplaceBack(component);

        return *component;
    }

    Component* SceneObject::GetComponent(TypeID typeId, u32 index)
    {
        if (auto it = m_components.Find(typeId))
        {
            if (index < it->second.instances.Size())
            {
                return it->second.instances[index];
            }
        }
        return nullptr;
    }

    void SceneObject::RemoveChild(const SceneObject* child)
    {
        if (child == nullptr) return;

        m_children.Erase(m_children.begin() + child->m_parentIndex,
                         m_children.begin() + child->m_parentIndex + 1);
    }

    void SceneObject::DoUpdate(f64 deltaTime)
    {
        m_updating = true;

        for(const auto& it: m_components)
        {
            for(Component* component : it.second.instances)
            {
                component->OnUpdate(deltaTime);
            }
        }

        for (SceneObject* child : m_children)
        {
            child->DoUpdate(deltaTime);
        }

        m_updating = false;
        if (m_markedToDestroy)
        {
            Destroy();
        }
    }

    void SceneObject::Destroy()
    {
        m_markedToDestroy = true;
        if (!m_updating)
        {
            if (m_parent)
            {
                m_parent->RemoveChild(this);
            }
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(this);
        }
    }

    void SceneObject::RegisterType(NativeTypeHandler<SceneObject>& type)
    {

    }
}
