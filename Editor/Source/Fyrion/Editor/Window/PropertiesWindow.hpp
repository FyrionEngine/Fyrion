#pragma once
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    struct SceneObject;

    class PropertiesWindow : public EditorWindow
    {
    public:
        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<PropertiesWindow>& type);
    private:
        String              m_StringCache{};
        bool				m_renamingFocus{};
        String				m_renamingCache{};
        RID                 m_renamingObject{};
        String              m_searchComponentString{};

        static void OpenProperties(VoidPtr userData);
        void        DrawSceneObject(u32 id, RID rid);
    };
}
