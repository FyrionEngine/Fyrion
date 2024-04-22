#include "SceneEditor.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Resource/AssetTree.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Scene/SceneManager.hpp"
#include "Fyrion/Scene/SceneAssets.hpp"

namespace Fyrion
{
    void SceneEditor::LoadScene(const RID rid)
    {
        m_count = 0;

        ResourceObject asset = Repository::Read(rid);
        m_rootObject = asset[Asset::Object].As<RID>();
    }

    RID SceneEditor::GetRootObject() const
    {
        //return m_rootObject;
        return m_rootObject;
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

        for (auto it : m_selectedObjects)
        {
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

    bool SceneEditor::IsSelected(RID object)
    {
        return m_selectedObjects.Has(object);
    }

    bool SceneEditor::IsParentOfSelected(RID object) const
    {
        for (const auto& it : m_selectedObjects)
        {
            // if (SceneObject* selectedObj = FindObjectByRID(it.first))
            // {
            //     if (selectedObj->GetParent() == object)
            //     {
            //         return true;
            //     }
            // }
        }
        return false;
    }

    bool SceneEditor::IsSimulating()
    {
        return false;
    }

    SceneObject* SceneEditor::GetLastSelectedObject() const
    {
        return nullptr;
    }

    void SceneEditor::AddComponent(SceneObject* object, TypeHandler* typeHandler)
    {
        RID component = Repository::CreateResource(typeHandler->GetTypeInfo().typeId);
        VoidPtr instance = typeHandler->NewInstance();
        Repository::Commit(component, instance);
        typeHandler->Destroy(instance);

        ResourceObject write = Repository::Write(object->GetAsset());
        write.AddToSubObjectSet(SceneObjectAsset::Components, component);
        write.Commit();
    }
}
