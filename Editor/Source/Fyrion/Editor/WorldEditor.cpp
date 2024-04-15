#include "WorldEditor.hpp"

#include "Editor.hpp"
#include "Fyrion/Resource/AssetTree.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceObject.hpp"
#include "Fyrion/World/WordAsset.hpp"

namespace Fyrion
{
    void WorldEditor::LoadWorld(RID rid)
    {
        m_worldAsset = rid;

        ResourceObject asset = Repository::Read(m_worldAsset);
        m_worldName     = asset[Asset::Name].As<String>();
        m_worldObject   = asset.GetSubObject(Asset::Object);

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
        return m_worldName;
    }

    RID WorldEditor::GetWorldObject() const
    {
        return m_worldObject;
    }

    void WorldEditor::CreateEntity(EditorEntity* parent)
    {
        RID entityRID = Repository::CreateResource<EntityAsset>();
        ResourceObject write = Repository::Write(entityRID);
        write.Commit();
        Repository::SetUUID(entityRID, UUID::RandomUUID());

        ResourceObject world = Repository::Write(m_worldObject);
        world.AddToSubObjectSet(WorldAsset::Entities, entityRID);
        world.Commit();

        //TODO make both dirty it automatically with events
        Editor::GetAssetTree().MarkDirty();
        m_dirty = true;
    }

    void WorldEditor::Update()
    {
        if (m_dirty)
        {
            m_entities.Clear();
            m_rootEntities.Clear();

            ResourceObject world = Repository::Read(m_worldObject);
            Array<RID> entities = world.GetSubObjectSetAsArray(WorldAsset::Entities);
            for(RID entity: entities)
            {
                u64 id = 0;

                if (auto it = m_ridIds.Find(entity))
                {
                    id = it->second;
                }
                else
                {
                    id = m_editorIdCount++;
                    m_ridIds[entity] = id;
                }

                EditorEntity* editorEntity = m_entities.Emplace(id, MakeUnique<EditorEntity>(id, entity)).first->second.Get();

                ResourceObject asset = Repository::Read(entity);
                if (asset.Has(EntityAsset::Name))
                {
                    editorEntity->name = asset[EntityAsset::Name].As<String>();
                }
                else
                {
                    editorEntity->name = "Entity " + ToString(m_entities.Size() - 1);
                }
                m_rootEntities.EmplaceBack(editorEntity);

            }
            m_dirty = false;
        }
    }

    Span<EditorEntity*> WorldEditor::GetRootEntities() const
    {
        return m_rootEntities;
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
