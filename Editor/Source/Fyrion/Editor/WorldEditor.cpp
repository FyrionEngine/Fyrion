#include "WorldEditor.hpp"

#include "Editor.hpp"
#include "Fyrion/Resource/AssetTree.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceObject.hpp"
#include "Fyrion/World/WordAsset.hpp"

namespace Fyrion
{
    void WorldEditor::LoadDefaultWorld()
    {

    }

    void WorldEditor::LoadWorld(RID rid)
    {
        m_worldAsset = rid;

        ResourceObject asset    = Repository::Read(m_worldAsset);
        m_rootEntity.name       = asset[Asset::Name].As<String>();
        m_worldObject           = asset.GetSubObject(Asset::Object);

        // if (Repository::GetResourceTypeID(assetObject) == GetTypeID<WorldAsset>())
        // {
        //     ResourceObject worldObject = Repository::Read(assetObject);
        // }
        m_dirty = true;
    }

    bool WorldEditor::IsLoaded() const
    {
        return m_worldObject;
    }

    StringView WorldEditor::GetWorldName() const
    {
        return m_rootEntity.name;
    }

    RID WorldEditor::GetWorldObject() const
    {
        return m_worldObject;
    }

    void WorldEditor::CreateEntity()
    {
        if (!m_worldObject) return;

        if (m_selectedEntities.Empty())
        {
            RID entityRID = Repository::CreateResource<EntityAsset>();
            ResourceObject write = Repository::Write(entityRID);
            write.Commit();
            Repository::SetUUID(entityRID, UUID::RandomUUID());

            ResourceObject world = Repository::Write(m_worldObject);
            world.AddToSubObjectSet(WorldAsset::Entities, entityRID);
            world.Commit();
        }
        else
        {
            for (auto it: m_selectedEntities)
            {
                if (const auto& itEntity = m_entities.Find(it.first))
                {
                    EditorEntity* parent = itEntity->second.Get();

                    RID entityRID = Repository::CreateResource<EntityAsset>();
                    ResourceObject write = Repository::Write(entityRID);
                    write[EntityAsset::Parent] = parent->rid;
                    write.Commit();
                    Repository::SetUUID(entityRID, UUID::RandomUUID());

                    ResourceObject writeParent = Repository::Write(parent->rid);
                    writeParent.AddToSubObjectSet(EntityAsset::Children, entityRID);
                    writeParent.Commit();
                }
            }
            m_selectedEntities.Clear();
        }

        //TODO make both dirty automatically with events
        Editor::GetAssetTree().MarkDirty();
        m_dirty = true;
    }

    void WorldEditor::DestroyEntity()
    {
        if (!m_worldObject) return;

        for (auto it: m_selectedEntities)
        {
            if (const auto& itEntity = m_entities.Find(it.first))
            {
                if (itEntity->second->rid)
                {
                    Repository::DestroyResource(itEntity->second->rid);
                }
            }
        }

        m_selectedEntities.Clear();

        Editor::GetAssetTree().MarkDirty();
        m_dirty = true;
    }

    void WorldEditor::Update()
    {
        if (m_dirty)
        {
            m_entities.Clear();
            m_rootEntity.children.Clear();

            ResourceObject world = Repository::Read(m_worldObject);
            UpdateChildren(&m_rootEntity, world.GetSubObjectSetAsArray(WorldAsset::Entities));

            m_dirty = false;
        }
    }

    void WorldEditor::UpdateChildren(EditorEntity* parentEntity, Span<RID> children)
    {
        for(RID entity: children)
        {
            u64 id = 0;

            if (const auto it = m_ridIds.Find(entity))
            {
                id = it->second;
            }
            else
            {
                id = m_editorIdCount++;
                m_ridIds[entity] = id;
            }

            EditorEntity* editorEntity = m_entities.Emplace(id, MakeUnique<EditorEntity>(id, entity)).first->second.Get();
            editorEntity->selected  = m_selectedEntities.Has(id);

            ResourceObject asset = Repository::Read(entity);
            if (asset.Has(EntityAsset::Name))
            {
                editorEntity->name = asset[EntityAsset::Name].As<String>();
            }
            else
            {
                editorEntity->name = "Entity " + ToString(id);
            }

            parentEntity->children.EmplaceBack(editorEntity);

            UpdateChildren(editorEntity, asset.GetSubObjectSetAsArray(EntityAsset::Children));
        }
    }

    EditorEntity* WorldEditor::GetRootEntity()
    {
        return &m_rootEntity;
    }

    void WorldEditor::CleanSelection()
    {
        for(auto& it : m_selectedEntities)
        {
            if (const auto& itEntity = m_entities.Find(it.first))
            {
                itEntity->second->selected = false;
            }
        }
        m_selectedEntities.Clear();
    }

    void WorldEditor::SelectEntity(EditorEntity* editorEntity)
    {
        editorEntity->selected = true;
        m_selectedEntities.Emplace(editorEntity->editorId);
    }


}
