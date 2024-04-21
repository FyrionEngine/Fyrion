#include "SceneEditor.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/Resource/AssetTree.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Scene/SceneManager.hpp"
#include "Fyrion/Scene/SceneTypes.hpp"

namespace Fyrion
{
    SceneEditor::SceneEditor()
    {
        Repository::AddResourceTypeEvent(GetTypeID<SceneObjectAsset>(), this, ResourceEventType::Update, SceneObjectAssetChanged);
    }

    SceneEditor::~SceneEditor()
    {
        Repository::RemoveResourceTypeEvent(GetTypeID<SceneObjectAsset>(), this, SceneObjectAssetChanged);
    }

    void SceneEditor::LoadScene(RID rid)
    {
        //m_objects.Clear();
        m_count = 0;
        m_rootObject = SceneManager::LoadScene(rid);
    }

    SceneObject* SceneEditor::GetRootObject() const
    {
        return m_rootObject;
    }

    SceneObject* SceneEditor::FindObjectByRID(RID rid) const
    {
        // if (auto it = m_objects.Find(rid))
        // {
        //     return it->second.Get();
        // }
        return nullptr;
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
            u64 size = root.GetSubObjectSetCount(SceneObjectAsset::Children);

            ResourceObject write = Repository::Write(entityRID);
            write[SceneObjectAsset::Name] = "Object " + ToString(m_count);
            write[SceneObjectAsset::Order] = size;
            write.Commit();

            Repository::SetUUID(entityRID, UUID::RandomUUID());

            root.AddToSubObjectSet(SceneObjectAsset::Children, entityRID);
            root.Commit();
        }
        else
        {
            HashSet<RID> selected = m_selectedObjects;
            ClearSelection();

            for (auto it: selected)
            {
                // if (const auto& itObject = m_objects.Find(it.first))
                // {
                //     SceneObject* parent = itObject->second.Get();
                //     ResourceObject writeParent = Repository::Write(parent->rid);
                //     u64 size = writeParent.GetSubObjectSetCount(SceneObjectAsset::Children);
                //
                //     RID objectRid = Repository::CreateResource<SceneObjectAsset>();
                //     m_selectedObjects.Emplace(objectRid);
                //
                //     ResourceObject write = Repository::Write(objectRid);
                //     write[SceneObjectAsset::Name] = "Object " + ToString(m_count);
                //     write[SceneObjectAsset::Order] = size;
                //     write[SceneObjectAsset::Parent] = parent->GetAsset();
                //     write.Commit();
                //
                //     Repository::SetUUID(objectRid, UUID::RandomUUID());
                //
                //     writeParent.AddToSubObjectSet(SceneObjectAsset::Children, objectRid);
                //     writeParent.Commit();
                // }
            }
        }

        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::DestroySelectedObjects()
    {
        if (m_rootObject == nullptr) return;

        for (auto it: m_selectedObjects)
        {
            // if (const auto& itObject = m_objects.Find(it.first))
            // {
            //     if (itObject->second->rid)
            //     {
            //         Repository::DestroyResource(itObject->second->rid);
            //     }
            // }
        }

        m_selectedObjects.Clear();
        Editor::GetAssetTree().MarkDirty();
    }

    void SceneEditor::ClearSelection()
    {
        for (auto& it : m_selectedObjects)
        {
            // if (const auto& itObject = m_objects.Find(it.first))
            // {
            //     itObject->second->selected = false;
            // }
        }
        m_selectedObjects.Clear();
        m_lastSelectedObject = nullptr;
    }

    void SceneEditor::SelectObject(SceneObject* object)
    {
        // object->selected = true;
        // m_selectedObjects.Emplace(object->GetAsset());
        // m_lastSelectedObject = object;
    }

    bool SceneEditor::IsSelected(SceneObject* object)
    {
        return false;
    }

    bool SceneEditor::IsParentOfSelected(SceneObject* object) const
    {
        // for (auto& it : m_selectedObjects)
        // {
        //     if (const auto& itObject = m_objects.Find(it.first))
        //     {
        //         if (itObject->second->parent == object)
        //         {
        //             return true;
        //         }
        //     }
        // }
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

    SceneObject* SceneEditor::LoadSceneObjectAsset(RID rid)
    {


        // SceneObject* object = m_objects.Emplace(rid, MakeUnique<SceneObject>(rid)).first->second.Get();
        // ResourceObject scene = Repository::Read(rid);
        // UpdateSceneObject(*object, scene);
        // m_count++;
        // return object;

        return nullptr;
    }


    void SceneEditor::UpdateSceneObject(SceneObject& object, ResourceObject& resource, bool updateChildren)
    {
        // object.children.Clear();
        // if (object.Has(SceneObjectAsset::Name))
        // {
        //     object.name = object[SceneObjectAsset::Name].As<String>();
        // }
        //
        // if (object.Has(SceneObjectAsset::Order))
        // {
        //     object.order = object[SceneObjectAsset::Order].As<u64>();
        // }
        //
        // if (m_selectedObjects.Has(object.rid) && !object.selected)
        // {
        //     object.selected = true;
        // }
        //
        // object.uuid = Repository::GetUUID(object.GetRID());
        //
        // Array<RID> children = object.GetSubObjectSetAsArray(SceneObjectAsset::Children);
        // for (RID child : children)
        // {
        //     SceneObject* childObject = FindObjectByRID(child);
        //     if (!childObject || updateChildren)
        //     {
        //         childObject = LoadSceneObjectAsset(child);
        //     }
        //     childObject->parent = &object;
        //     object.children.EmplaceBack(childObject);
        // }
        //
        // Sort(object.children.begin(), object.children.end(), [](SceneObject* left, SceneObject* right)
        // {
        //     return left->order < right->order;
        // });
    }

    void SceneEditor::SceneObjectAssetChanged(VoidPtr userData, ResourceEventType eventType, ResourceObject& oldObject, ResourceObject& newObject)
    {
        SceneEditor* sceneEditor = static_cast<SceneEditor*>(userData);
        SceneObject* object = sceneEditor->FindObjectByRID(newObject.GetRID());
        if (object)
        {
            sceneEditor->UpdateSceneObject(*object, newObject, false);
        }
    }
}
