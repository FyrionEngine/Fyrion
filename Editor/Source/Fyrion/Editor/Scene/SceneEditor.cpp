#include "SceneEditor.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Resource/AssetTree.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Scene/SceneManager.hpp"
#include "Fyrion/Scene/SceneAssets.hpp"

namespace Fyrion
{
    SceneEditor::SceneEditor()
    {
        Repository::AddResourceTypeEvent(GetTypeID<SceneObjectAsset>(), this,
                                         ResourceEventType::Insert | ResourceEventType::Update | ResourceEventType::Destroy
                                         , SceneObjectAssetChanged);
    }

    SceneEditor::~SceneEditor()
    {
        Repository::RemoveResourceTypeEvent(GetTypeID<SceneObjectAsset>(), this, SceneObjectAssetChanged);
    }

    void SceneEditor::LoadScene(const RID rid)
    {
        m_loading = true;
        m_rootObject = SceneManager::LoadScene(rid);
        m_loading = false;
    }

    SceneObject* SceneEditor::GetRootObject() const
    {
        return m_rootObject;
    }

    SceneObject* SceneEditor::FindObjectByRID(const RID rid) const
    {
        return m_rootObject->GetSceneGlobals()->FindByRID(rid);
    }

    bool SceneEditor::IsLoaded() const
    {
        return m_rootObject != nullptr;
    }

    void SceneEditor::CreateObject()
    {
        if (m_rootObject == nullptr) return;

        if (m_selectedObjects.Empty())
        {
            RID entityRID = Repository::CreateResource<SceneObjectAsset>();

            ResourceObject root = Repository::Write(m_rootObject->GetAsset());
            u64            size = root.GetSubObjectSetCount(SceneObjectAsset::Children);

            ResourceObject write = Repository::Write(entityRID);
            write[SceneObjectAsset::Name] = "Object " + ToString(m_count);
            write[SceneObjectAsset::Order] = size;
            write[SceneObjectAsset::Parent] = m_rootObject->GetAsset();
            write.Commit();

            Repository::SetUUID(entityRID, UUID::RandomUUID());

            root.AddToSubObjectSet(SceneObjectAsset::Children, entityRID);
            root.Commit();
        }
        else
        {
            HashSet<RID> selected = m_selectedObjects;
            ClearSelection();

            for (const auto& it : selected)
            {
                if (SceneObject* parent = FindObjectByRID(it.first))
                {
                    ResourceObject writeParent = Repository::Write(parent->GetAsset());
                    u64            size = writeParent.GetSubObjectSetCount(SceneObjectAsset::Children);

                    RID objectRid = Repository::CreateResource<SceneObjectAsset>();
                    m_selectedObjects.Emplace(objectRid);

                    ResourceObject write = Repository::Write(objectRid);
                    write[SceneObjectAsset::Name] = "Object " + ToString(m_count);
                    write[SceneObjectAsset::Order] = size;
                    write[SceneObjectAsset::Parent] = parent->GetAsset();
                    write.Commit();

                    Repository::SetUUID(objectRid, UUID::RandomUUID());

                    writeParent.AddToSubObjectSet(SceneObjectAsset::Children, objectRid);
                    writeParent.Commit();
                }
            }
        }
        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::DestroySelectedObjects()
    {
        if (m_rootObject == nullptr) return;

        for (auto it : m_selectedObjects)
        {
            if (SceneObject* object = FindObjectByRID(it.first))
            {
                if (m_rootObject != object && object->GetAsset())
                {
                    Repository::DestroyResource(object->GetAsset());
                }
            }
        }

        m_selectedObjects.Clear();
        m_lastSelectedObject = nullptr;
        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::ClearSelection()
    {
        m_selectedObjects.Clear();
        m_lastSelectedObject = nullptr;
    }

    void SceneEditor::SelectObject(SceneObject* object)
    {
        m_selectedObjects.Emplace(object->GetAsset());
        m_lastSelectedObject = object;
    }

    bool SceneEditor::IsSelected(SceneObject* object)
    {
        return m_selectedObjects.Has(object->GetAsset());
    }

    bool SceneEditor::IsParentOfSelected(SceneObject* object) const
    {
        for (const auto& it : m_selectedObjects)
        {
            if (SceneObject* selectedObj = FindObjectByRID(it.first))
            {
                if (selectedObj->GetParent() == object)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool SceneEditor::IsSimulating()
    {
        return false;
    }

    SceneObject* SceneEditor::GetLastSelectedObject() const
    {
        return m_lastSelectedObject;
    }

    //TODO - this is a bit strange, needs to be refactored
    void SceneEditor::SceneObjectAssetChanged(VoidPtr userData, ResourceEventType eventType, ResourceObject& oldObject, ResourceObject& newObject)
    {
        SceneEditor* sceneEditor = static_cast<SceneEditor*>(userData);

        if (eventType == ResourceEventType::Insert)
        {
            sceneEditor->m_count++;
        }

        if (sceneEditor->m_rootObject)
        {
            if (!sceneEditor->m_loading)
            {
                if (eventType == ResourceEventType::Insert)
                {
                    if (SceneObject* object = sceneEditor->FindObjectByRID(newObject[SceneObjectAsset::Parent].As<RID>()))
                    {
                        object->NewChild(newObject.GetRID());
                    }
                }
                else if (eventType == ResourceEventType::Destroy)
                {
                    if (SceneObject* object = sceneEditor->FindObjectByRID(oldObject.GetRID()))
                    {
                        object->Destroy();
                    }
                }
            }
        }
    }
}
