#include "SceneObject.hpp"

#include "SceneManager.hpp"
#include "Assets/SceneObjectAsset.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "SceneTypes.hpp"

namespace Fyrion
{
    SceneObject::SceneObject() {}

    SceneObject::SceneObject(SceneObjectAsset* asset) : root(true), asset(asset) {}

    SceneObject::~SceneObject()
    {
        for (SceneObject* child : children)
        {
            MemoryGlobals::GetDefaultAllocator().DestroyAndFree(child);
        }
    }

    Component& SceneObject::AddComponent(TypeID typeId)
    {
        return AddComponent(Registry::FindTypeById(typeId));
    }

    Component& SceneObject::AddComponent(TypeHandler* typeHandler)
    {
        Component* component = typeHandler->Cast<Component>(typeHandler->NewInstance());
        component->typeHandler = typeHandler;
        component->object = this;
        components.EmplaceBack(component);

        if (!notificationDisabled)
        {
            TypeID typeId = typeHandler->GetTypeInfo().typeId;
            Notify(SceneNotifications_OnComponentAdded, &typeId);
            component->OnNotify(SceneNotifications_OnComponentCreated, &typeId);
        }

        return *component;
    }

    Component& SceneObject::CloneComponent(const Component* originComponent)
    {
        TypeHandler* typeHandler = originComponent->typeHandler;
        Component* component = typeHandler->Cast<Component>(typeHandler->NewInstance());
        typeHandler->DeepCopy(originComponent, component);

        component->typeHandler = typeHandler;
        component->object = this;
        components.EmplaceBack(component);


        if (!notificationDisabled)
        {
            TypeID typeId = typeHandler->GetTypeInfo().typeId;
            Notify(SceneNotifications_OnComponentAdded, &typeId);
            component->OnNotify(SceneNotifications_OnComponentCreated, &typeId);
        }
        return *component;
    }

    void SceneObject::RemoveComponent(Component* component)
    {
        usize index = components.IndexOf(component);
        if (index != nPos)
        {
            components.Remove(index);
        }
    }

    Span<Component*> SceneObject::GetComponents() const
    {
        return components;
    }

    Component* SceneObject::GetComponent(TypeID typeId) const
    {
        for (Component* component : components)
        {
            if (component->typeHandler->GetTypeInfo().typeId == typeId)
            {
                return component;
            }
        }

        return nullptr;
    }

    Component* SceneObject::FindComponentByPrototype(const UUID& p_prototype) const
    {
        for (Component* component : components)
        {
            if (component->GetPrototype() == p_prototype)
            {
                return component;
            }
        }
        return nullptr;
    }

    Component* SceneObject::FindComponentByUUID(const UUID& p_uuid) const
    {
        for (Component* component : components)
        {
            if (component->GetUUID() == p_uuid)
            {
                return component;
            }
        }
        return nullptr;
    }

    StringView SceneObject::GetName() const
    {
        if (asset)
        {
            return asset->GetName();
        }

        return name;
    }

    void SceneObject::SetName(const StringView& p_name)
    {
        if (asset)
        {
            asset->SetName(p_name);
        }
        else
        {
            name = p_name;
        }
    }

    SceneObject* SceneObject::GetParent() const
    {
        return parent;
    }

    Span<SceneObject*> SceneObject::GetChildren() const
    {
        return children;
    }

    SceneObject* SceneObject::FindChildByName(const StringView& p_name) const
    {
        //TODO improve O(n)
        for (SceneObject* child : children)
        {
            if (child->GetName() == p_name)
            {
                return child;
            }
        }
        return nullptr;
    }

    SceneObject* SceneObject::FindChildByUUID(const UUID& p_uuid) const
    {
        //TODO improve O(n)
        for (SceneObject* child : children)
        {
            if (child->GetUUID() == p_uuid)
            {
                return child;
            }
        }
        return nullptr;
    }

    SceneObject* SceneObject::FindChildByPrototype(const UUID& p_prototype) const
    {
        //TODO improve O(n)
        for (SceneObject* child : children)
        {
            if (child->prototypeUUID == p_prototype)
            {
                return child;
            }
        }
        return nullptr;
    }

    void SceneObject::SetUUID(UUID p_uuid)
    {
        uuid = p_uuid;
    }

    UUID SceneObject::GetUUID() const
    {
        if (uuid)
        {
            return uuid;
        }

        if (asset)
        {
            return asset->GetUUID();
        }

        return {};
    }

    SceneObject* SceneObject::GetPrototype() const
    {
        return prototype;
    }

    void SceneObject::AddChild(SceneObject* sceneObject)
    {
        sceneObject->parent = this;
        children.EmplaceBack(sceneObject);
    }

    void SceneObject::AddChildAt(SceneObject* sceneObject, usize pos)
    {
        sceneObject->parent = this;
        children.Insert(children.begin() + pos, &sceneObject, &sceneObject + 1);
    }

    void SceneObject::RemoveChild(SceneObject* sceneObject)
    {
        if (const auto it = FindFirst(children.begin(), children.end(), sceneObject))
        {
            children.Erase(it);
        }
    }

    void SceneObject::RemoveChildAt(usize pos)
    {
        children.Remove(pos);
    }

    void SceneObject::Destroy()
    {
        for (SceneObject* child : children)
        {
            child->Destroy();
        }
        SceneManager::Destroy(this);
        children.Clear();
    }

    void SceneObject::RegisterType(NativeTypeHandler<SceneObject>& type)
    {
        type.Constructor<SceneObjectAsset*>();

        type.Field<&SceneObject::name>("name");
    }

    ArchiveObject SceneObject::Serialize(ArchiveWriter& writer) const
    {
        if (!GetUUID())
        {
            return {};
        }

        ArchiveObject object = writer.CreateObject();

        if (!name.Empty() && (prototype == nullptr || name != prototype->GetName()))
        {
            writer.WriteString(object, "name", name);
        }

        if (uuid)
        {
            writer.WriteString(object, "uuid", ToString(uuid));
        }

        if (prototype)
        {
            writer.WriteString(object, "prototype", ToString(prototype->GetUUID()));
        }

        ArchiveObject objChildren{};
        for (const SceneObject* child : children)
        {
            if (!prototype || child->HasPrototypeOverride())
            {
                if (ArchiveObject childObject = child->Serialize(writer))
                {
                    if (!objChildren)
                    {
                        objChildren = writer.CreateArray();
                    }
                    writer.AddValue(objChildren, childObject);
                }
            }
        }

        if (objChildren)
        {
            writer.WriteValue(object, "children", objChildren);
        }

        ArchiveObject objComponents{};

        for (const Component* component : components)
        {

            if (!prototype || IsComponentOverride(component))
            {
                ArchiveObject componentObject = Serialization::Serialize(component->typeHandler, writer, component);
                writer.WriteString(componentObject, "_type", component->typeHandler->GetName());
                if (component->GetUUID())
                {
                    writer.WriteString(componentObject, "_uuid", ToString(component->GetUUID()));
                }

                if (component->GetPrototype())
                {
                    writer.WriteString(componentObject, "_prototype", ToString(component->GetPrototype()));
                }

                if (!objComponents)
                {
                    objComponents = writer.CreateArray();
                }
                writer.AddValue(objComponents, componentObject);
            }
        }

        if (objComponents)
        {
            writer.WriteValue(object, "components", objComponents);
        }

        return object;
    }

    void SceneObject::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        notificationDisabled = true;

        name = reader.ReadString(object, "name");
        uuid = UUID::FromString(reader.ReadString(object, "uuid"));

        if (ArchiveObject childrenArr = reader.ReadObject(object, "children"))
        {
            usize childrenArrSize = reader.ArrSize(childrenArr);

            ArchiveObject childObject{};
            for (usize i = 0; i < childrenArrSize; ++i)
            {
                childObject = reader.Next(childrenArr, childObject);

                SceneObject* child = SceneManager::CreateObject();
                child->Deserialize(reader, childObject);
                AddChild(child);
            }
        }

        if (ArchiveObject compArr = reader.ReadObject(object, "components"))
        {
            usize compArrSize = reader.ArrSize(compArr);

            ArchiveObject compObj{};
            for (usize i = 0; i < compArrSize; ++i)
            {
                compObj = reader.Next(compArr, compObj);

                if (TypeHandler* typeHandler = Registry::FindTypeByName(reader.ReadString(compObj, "_type")))
                {
                    Component& component = AddComponent(typeHandler);
                    component.SetUUID(UUID::FromString(reader.ReadString(compObj, "_uuid")));
                    component.SetPrototype(UUID::FromString(reader.ReadString(compObj, "_prototype")));
                    Serialization::Deserialize(typeHandler, reader, compObj, &component);
                }
            }
        }

        prototypeUUID = UUID::FromString(reader.ReadString(object, "prototype"));
        if (SceneObjectAsset* asset = AssetDatabase::FindById<SceneObjectAsset>(prototypeUUID))
        {
            SetPrototype(asset->GetObject());
        }

        notificationDisabled = false;
    }

    bool SceneObject::IsAlive() const
    {
        return alive;
    }

    void SceneObject::SetAlive(bool p_alive)
    {
        alive = p_alive;
        Notify(alive ? SceneNotifications_OnActivate : SceneNotifications_OnDeactivate, nullptr);
    }

    void SceneObject::Notify(i64 type, VoidPtr userData)
    {
        for (Component* component : components)
        {
            component->OnNotify(type, userData);
        }

        for (SceneObject* child : children)
        {
            child->Notify(type, userData);
        }
    }

    SceneObject* SceneObject::Clone() const
    {
        SceneObject* object = MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>();
        object->SetUUID(UUID::RandomUUID());
        object->SetName(GetName());
        object->prototype = prototype;
        object->asset = asset;

        for (const SceneObject* child : children)
        {
            object->AddChild(child->Clone());
        }

        for (const Component* component : components)
        {
            object->CloneComponent(component);
        }

        return object;
    }

    void SceneObject::SetPrototype(SceneObject* p_prototype)
    {
        prototype = p_prototype;
        prototypeUUID = p_prototype->GetUUID();

        SetName(p_prototype->GetName());
        SetUUID(UUID::RandomUUID());

        Array<SceneObject*> childrenToAdd;
        childrenToAdd.Reserve(p_prototype->children.Size());

        for (SceneObject* child : p_prototype->children)
        {
            if (SceneObject* childPrototype = FindChildByPrototype(child->GetUUID()))
            {
                //to keep the original order, not exactly the most performact way, but does the job
                //maybe needs to be revisited later
                this->RemoveChild(childPrototype);
                childrenToAdd.EmplaceBack(childPrototype);

                childPrototype->SetPrototype(child);

                continue;
            }

            SceneObject* newChild = MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>();
            newChild->SetPrototype(child);
            childrenToAdd.EmplaceBack(newChild);
        }

        children.Reserve(children.Size() + childrenToAdd.Size());

        for(SceneObject* newChild : childrenToAdd)
        {
            this->AddChild(newChild);
        }

        for (const Component* component : p_prototype->components)
        {
            if (FindComponentByPrototype(component->GetUUID()))
            {
                componentOverride.Insert(component->GetUUID());
                continue;
            }

            Component& newComponent = this->CloneComponent(component);
            newComponent.SetPrototype(component->GetUUID());
        }
    }

    void SceneObject::OverridePrototypeComponent(const Component* component)
    {
        if(!component || !component->GetPrototype()) return;
        componentOverride.Insert(component->GetPrototype());
    }

    bool SceneObject::IsComponentOverride(const Component* component) const
    {
        if(!component || !component->GetPrototype()) return false;
        return componentOverride.Has(component->GetPrototype());
    }

    void SceneObject::RemoveOverridePrototypeComponent(Component* component)
    {
        if (!component || !component->GetPrototype()) return;
        componentOverride.Erase(component->GetPrototype());

        if (Component* originalValue = prototype->FindComponentByUUID(component->GetPrototype()))
        {
            component->typeHandler->DeepCopy(originalValue, component);
            component->OnChange();
        }
    }

    bool SceneObject::HasPrototypeOverride() const
    {
        if (!componentOverride.Empty())
        {
            return true;
        }

        if (!prototype)
        {
            return true;
        }

        if (name != prototype->GetName())
        {
            return true;
        }

        for (SceneObject* child : children)
        {
            if (child->HasPrototypeOverride())
            {
                return true;
            }
        }

        return false;
    }
}
