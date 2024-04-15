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

        void       LoadWorld(RID rid);
        bool       IsLoaded() const;
        StringView GetWorldName() const;
        RID        GetWorldObject() const;
        void       CreateEntity(EditorEntity* parent = nullptr);
        void       Update();

        void       CleanSelection();
        void       SelectEntity(EditorEntity* editorEntity);

        Span<EditorEntity*> GetRootEntities() const;

    private:
        bool   m_dirty = false;
        String m_worldName;
        RID    m_worldAsset;
        RID    m_worldObject;
        u64    m_editorIdCount{};

        HashMap<RID, u64>                       m_ridIds{};
        HashMap<u64, UniquePtr<EditorEntity>>   m_entities{};
        Array<EditorEntity*>                    m_rootEntities{};
        HashSet<u64>                            m_selectedEntities{};
    };
}
