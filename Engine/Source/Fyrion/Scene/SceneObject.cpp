#include "SceneObject.hpp"

#include "Component.hpp"
#include "SceneManager.hpp"
#include "SceneAssets.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{
    SceneObjectIterator::SceneObjectIterator(SceneObject* object) : m_object(object)
    {
    }

    SceneObjectIterator SceneObjectIterator::begin() const
    {
        return {m_object->m_head};
    }

    SceneObjectIterator SceneObjectIterator::end() const
    {
        return {nullptr};
    }

    SceneObject& SceneObjectIterator::operator*() const
    {
        return *m_object;
    }

    SceneObject* SceneObjectIterator::operator->() const
    {
        return m_object;
    }

    bool operator==(const SceneObjectIterator& a, const SceneObjectIterator& b)
    {
        return a.m_object == b.m_object;
    }

    bool operator!=(const SceneObjectIterator& a, const SceneObjectIterator& b)
    {
        return !(a == b);
    }

    SceneObjectIterator& SceneObjectIterator::operator++()
    {
        m_object = m_object->m_next;
        return *this;
    }

    SceneObject::SceneObject(StringView name, SceneObject* parent) : m_name(name), m_parent(parent), m_sceneGlobals(m_parent->m_sceneGlobals)
    {
    }

    SceneObject::SceneObject(RID asset, SceneObject* parent) : m_asset(asset), m_parent(parent), m_sceneGlobals(m_parent->m_sceneGlobals)
    {
        // ResourceObject resource = Repository::Read(asset);
        // if (name.Empty() && resource.Has(SceneObjectAsset::Name))
        // {
        //     m_name = resource[SceneObjectAsset::Name].As<String>();
        // }
        //
        // if (resource.Has(SceneObjectAsset::Order))
        // {
        //     m_order = resource[SceneObjectAsset::Order].As<u64>();
        // }
        //
        // Array<RID> children = resource.GetSubObjectSetAsArray(SceneObjectAsset::Children);
        // m_children.Resize(children.Size());
        // for (RID child : children)
        // {
        //     SceneObject* childObject = MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>("", child, this);
        //     m_children[childObject->m_order] = childObject;
        // }
    }

    SceneObject::SceneObject(StringView name, RID asset, SceneGlobals* sceneGlobals) : m_name(name), m_asset(asset), m_sceneGlobals(sceneGlobals)
    {
    }

    SceneObject::~SceneObject()
    {
        for (auto it : m_components)
        {
            for (ComponentInstace& component : it.second.instances)
            {
                component.instance->OnDestroy();
                component.instance->SetUpdateEnabled(false);
                it.second.typeHandler->Destroy(component.instance);
            }
        }

        SceneObject* current = m_head;
        while(current != nullptr)
        {
            SceneObject* next = current->m_next;
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(current);
            current = next;
        }
    }

    SceneObject* SceneObject::NewChild(const StringView& name)
    {
        SceneObject* sceneObject = MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>(name, this);
        AddChild(sceneObject);
        return sceneObject;
    }

    SceneObject* SceneObject::NewChild(const RID& asset)
    {
        SceneObject* sceneObject = MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>(asset, this);
        AddChild(sceneObject);
        return sceneObject;
    }

    SceneObject* SceneObject::Duplicate() const
    {
        if (m_parent)
        {
            SceneObject* sceneObject = m_parent->NewChild(m_name);
            //TODO;
            return sceneObject;
        }
        FY_ASSERT(false, "Root entity cannot be duplicated");
        return nullptr;
    }

    u64 SceneObject::GetChildrenCount() const
    {
        return m_count;
    }

    SceneObjectIterator SceneObject::GetChildren()
    {
        return {this};
    }

    SceneGlobals* SceneObject::GetSceneGlobals() const
    {
        return m_sceneGlobals;
    }

    SceneObject* SceneObject::GetParent() const
    {
        return m_parent;
    }

    StringView SceneObject::GetName() const
    {
        return m_name;
    }

    void SceneObject::SetName(const StringView& newName)
    {
        m_name = newName;
    }

    RID SceneObject::GetAsset() const
    {
        return m_asset;
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
        if (m_head == child)
        {
            m_head = child->m_next;
        }

        if (m_tail == child)
        {
            m_tail = child->m_prev;
        }

        if (child->m_prev)
        {
            child->m_prev->m_next = child->m_next;
        }

        if (child->m_next)
        {
            child->m_next->m_prev = child->m_prev;
        }
        m_count--;
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

            for (SceneObject& sceneObject : GetChildren())
            {
                sceneObject.DoStart();
            }
            m_componentDirty = false;
        }
    }

    void SceneObject::DoUpdate(f64 deltaTime)
    {
        for (auto& it : m_components)
        {
            for (usize i = 0; i < it.second.instances.Size(); ++i)
            {
                it.second.instances[i].instance->OnUpdate(deltaTime);
            }
        }

        for (SceneObject& sceneObject : GetChildren())
        {
            sceneObject.DoUpdate(deltaTime);
        }
    }

    void SceneObject::Destroy()
    {
        if (!m_markedToDestroy)
        {
            m_markedToDestroy = true;
            GetSceneGlobals()->EnqueueDestroy(this);
        }
    }

    void SceneObject::AddChild(SceneObject* object)
    {
        if (!m_head)
        {
            m_head = object;
        }
        if (m_tail)
        {
            object->m_prev = m_tail;
            m_tail->m_next = object;
        }
        m_tail = object;
        m_count++;
    }

    void SceneObject::DestroyImmediate()
    {
        if (m_parent)
        {
            m_parent->RemoveChild(this);
        }
        MemoryGlobals::GetDefaultAllocator().DestroyAndFree(this);
    }

    void SceneObject::RegisterType(NativeTypeHandler<SceneObject>& type)
    {
    }
}
