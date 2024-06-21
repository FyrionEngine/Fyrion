#include "SceneEditor.hpp"

#include "Fyrion/Editor/Editor.hpp"

namespace Fyrion
{
#if FY_ASSET_REFACTOR
    void SceneEditor::LoadScene(const RID rid)
    {
        m_asset = rid;

        ResourceObject asset = Repository::Read(rid);
        m_rootObject = asset[Asset::Object].Value<RID>();
        m_rootName = asset[Asset::Name].Value<String>();

        m_count = SubObjectCount(m_rootObject); //TODO(Fyrion) : maybe m_count storing it will be better.
    }

    u64 SceneEditor::SubObjectCount(RID rid)
    {
        u64 count{};

        ResourceObject object = Repository::Read(rid);
        if (object.Has(SceneObjectAsset::Children))
        {
            Array<RID> children = object.GetSubObjectSetAsArray(SceneObjectAsset::Children);
            count = children.Size();

            for (RID child : children)
            {
                count += SubObjectCount(child);
            }
        }
        return count;
    }

    RID SceneEditor::GetRootObject() const
    {
        return m_rootObject;
    }

    StringView SceneEditor::GetRootName() const
    {
        ResourceObject asset = Repository::Read(m_asset);
        return asset[Asset::Name].Value<StringView>();
    }

    bool SceneEditor::IsLoaded() const
    {
        return m_rootObject;
    }

    void SceneEditor::CreateObject()
    {
        if (!m_rootObject) return;

        if (m_selectedObjects.Empty())
        {
            RID object = Repository::CreateResource<SceneObjectAsset>();

            ResourceObject root = Repository::Write(m_rootObject);

            ResourceObject write = Repository::Write(object);
            write[SceneObjectAsset::Name] = "Object " + ToString(m_count++);
            write.Commit();

            Repository::SetUUID(object, UUID::RandomUUID());

            Array<RID> children = root[SceneObjectAsset::ChildrenSort].Value<Array<RID>>();
            children.EmplaceBack(object);
            root[SceneObjectAsset::ChildrenSort] = children;

            root.AddToSubObjectSet(SceneObjectAsset::Children, object);
            root.Commit();
        }
        else
        {
            HashSet<RID> selected = m_selectedObjects;
            ClearSelection();

            for (const auto& it : selected)
            {
                ResourceObject writeParent = Repository::Write(it.first);

                RID object = Repository::CreateResource<SceneObjectAsset>();
                m_selectedObjects.Emplace(object);
                m_lastSelectedRid = object;

                ResourceObject write = Repository::Write(object);
                write[SceneObjectAsset::Name] = "Object " + ToString(m_count++);
                write.Commit();

                Repository::SetUUID(object, UUID::RandomUUID());

                Array<RID> children = writeParent[SceneObjectAsset::ChildrenSort].Value<Array<RID>>();
                children.EmplaceBack(object);
                writeParent[SceneObjectAsset::ChildrenSort] = children;
                writeParent.AddToSubObjectSet(SceneObjectAsset::Children, object);
                writeParent.Commit();
            }
        }
        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::DestroySelectedObjects()
    {
        if (!m_rootObject) return;

        for (const auto& it : m_selectedObjects)
        {
            if (m_rootObject == it.first) continue;

            if (RID parent = Repository::GetParent(it.first))
            {
                ResourceObject writeParent = Repository::Write(parent);
                Array<RID> children = writeParent[SceneObjectAsset::ChildrenSort].Value<Array<RID>>();
                auto itArr = FindFirst(children.begin(), children.end(), it.first);
                if (itArr)
                {
                    children.Erase(itArr, itArr + 1);
                }
                writeParent[SceneObjectAsset::ChildrenSort] = children;
                writeParent.Commit();
            }
            Repository::DestroyResource(it.first);
        }

        m_selectedObjects.Clear();
        m_lastSelectedRid = {};

        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::ClearSelection()
    {
        m_selectedObjects.Clear();
        m_lastSelectedRid = {};
    }

    void SceneEditor::SelectObject(RID object)
    {
        m_selectedObjects.Emplace(object);
        m_lastSelectedRid = object;
    }

    bool SceneEditor::IsSelected(RID object) const
    {
        return m_selectedObjects.Has(object);
    }

    bool SceneEditor::IsParentOfSelected(RID object) const
    {
        for (const auto& it : m_selectedObjects)
        {
            if (Repository::GetParent(it.first) == object)
            {
                return true;
            }
        }
        return false;
    }

    bool SceneEditor::IsSimulating()
    {
        return false;
    }

    void SceneEditor::RenameObject(RID rid, const StringView& newName)
    {

    }

    RID SceneEditor::GetLastSelectedObject() const
    {
        return m_lastSelectedRid;
    }

    void SceneEditor::AddComponent(RID object, TypeHandler* typeHandler)
    {
        RID component = Repository::CreateResource(typeHandler->GetTypeInfo().typeId);
        VoidPtr instance = typeHandler->NewInstance();
        Repository::Commit(component, instance);
        typeHandler->Destroy(instance);

        ResourceObject write = Repository::Write(object);
        write.AddToSubObjectSet(SceneObjectAsset::Components, component);
        write.Commit();

        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::RemoveComponent(RID object, RID component)
    {
        Repository::DestroyResource(component);

        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::ResetComponent(RID component)
    {
        Repository::Commit(component, nullptr);
        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::UpdateComponent(RID component, ConstPtr value)
    {
        Repository::Commit(component, value);
        Editor::GetAssetTree().MarkDirty();
    }
#endif

}
