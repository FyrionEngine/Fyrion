#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/World/WorldTypes.hpp"

namespace Fyrion
{
    struct EditorEntity
    {
        RID                  rid{};
        Entity               entity{};
        String               name{};
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

        Span<EditorEntity*> GetRootEntities() const;

    private:
        bool   m_dirty = false;
        String m_worldName;
        RID    m_worldAsset;
        RID    m_worldObject;

        HashMap<RID, UniquePtr<EditorEntity>> m_entities{};
        Array<EditorEntity*> m_rootEntities{};
    };
}
