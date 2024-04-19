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
            for (ComponentInstace& component : it.second.instances)
            {
                component.instance->OnDestroy();
                it.second.typeHandler->Destroy(component.instance);
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

    SceneObject* SceneObject::Duplicate() const
    {
        if(m_parent)
        {
            SceneObject* sceneObject = m_parent->NewChild(m_asset, m_name);

            //TODO;

            return sceneObject;
        }
        FY_ASSERT(false, "Root entity cannot be duplicated");
        return nullptr;
    }

    Span<SceneObject*> SceneObject::GetChildren() const
    {
        return m_children;
    }

    SceneObject* SceneObject::GetScene()
    {
        if (m_parent)
        {
            return m_parent->GetScene();
        }
        return this;
    }

    SceneObject* SceneObject::GetParent() const
    {
        return m_parent;
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

        SetComponentDirty();

        return *component;
    }

    void SceneObject::RemoveComponent(TypeID typeId, u32 index)
    {
        if (auto it = m_components.Find(typeId))
        {
            if (index < it->second.instances.Size())
            {
                ComponentInstace& component = it->second.instances[index];
                component.instance->OnDestroy();
                it->second.typeHandler->Destroy(component.instance);

                it->second.instances.Erase(
                    it->second.instances.begin() + index,
                    it->second.instances.begin() + index + 1
                );
            }
        }
    }

    Component* SceneObject::GetComponent(TypeID typeId, u32 index)
    {
        if (auto it = m_components.Find(typeId))
        {
            if (index < it->second.instances.Size())
            {
                return it->second.instances[index].instance;
            }
        }
        return nullptr;
    }

    u32 SceneObject::GetComponentTypeCount(TypeID typeId) const
    {
        if (const auto it = m_components.Find(typeId))
        {
            return it->second.instances.Size();
        }

        return 0;
    }

    u32 SceneObject::GetComponentCount() const
    {
        u32 total{};
        for(const auto& it: m_components)
        {
            total += it.second.instances.Size();
        }
        return total;
    }

    void SceneObject::RemoveChild(const SceneObject* child)
    {
        if (child == nullptr) return;

        m_children.Erase(m_children.begin() + child->m_parentIndex,
                         m_children.begin() + child->m_parentIndex + 1);
    }

    void SceneObject::SetComponentDirty()
    {
        m_componentDirty = true;
        if (m_parent && !m_parent->m_componentDirty)
        {
            m_parent->SetComponentDirty();
        }
    }

    void SceneObject::DoStart()
    {
        if (m_componentDirty)
        {
            for (auto& it : m_components)
            {
                for (usize i = 0; i < it.second.instances.Size(); ++i)
                {
                    ComponentInstace& component = it.second.instances[i];
                    if (!component.startCalled)
                    {
                        component.startCalled = true;
                        component.instance->OnStart();
                    }
                }
            }

            for (usize i = 0; i < m_children.Size(); ++i)
            {
                m_children[i]->DoStart();
            }

            m_componentDirty = false;
        }
    }

    void SceneObject::DoUpdate(f64 deltaTime)
    {
        m_updating = true;

        for (auto& it : m_components)
        {
            for (usize i = 0; i < it.second.instances.Size(); ++i)
            {
                it.second.instances[i].instance->OnUpdate(deltaTime);
            }
        }

        for (usize i = 0; i < m_children.Size(); ++i)
        {
            m_children[i]->DoUpdate(deltaTime);
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
