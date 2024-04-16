#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/World/WorldTypes.hpp"

namespace Fyrion
{
    struct EditorEntity
    {
        u64                  editorId{};
        RID                  rid{};
        Entity               entity{};
        String               name{};
        bool                 selected{};
        Array<EditorEntity*> children{};
    };

    class FY_API WorldEditor
    {
    public:
        WorldEditor() = default;
        WorldEditor(const WorldEditor&) = delete;
        WorldEditor& operator=(const WorldEditor& world) = delete;

        void       LoadDefaultWorld();
        void       LoadWorld(RID rid);
        bool       IsLoaded() const;
        StringView GetWorldName() const;
        RID        GetWorldObject() const;
        void       CreateEntity();
        void       DestroyEntity();
        void       Update();

        void       CleanSelection();
        void       SelectEntity(EditorEntity* editorEntity);

        EditorEntity* GetRootEntity();

    private:
        bool   m_dirty = false;
        RID    m_worldAsset;
        RID    m_worldObject;
        u64    m_editorIdCount{};

        EditorEntity                            m_rootEntity{};
        HashMap<RID, u64>                       m_ridIds{};
        HashMap<u64, UniquePtr<EditorEntity>>   m_entities{};
        HashSet<u64>                            m_selectedEntities{};

        void UpdateChildren(EditorEntity* editorEntity, Span<RID> children);
    };
}
