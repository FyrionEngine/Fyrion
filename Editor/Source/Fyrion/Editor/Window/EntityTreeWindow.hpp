#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/MenuItem.hpp"

namespace Fyrion
{
    struct EditorEntity;

    class FY_API EntityTreeWindow : public EditorWindow
    {
    public:
        void Draw(u32 id, bool& open) override;


        static void AddMenuItem(const MenuItemCreation& menuItem);

        static void RegisterType(NativeTypeHandler<EntityTreeWindow>& type);
    private:
        static void     OpenEntityTree(VoidPtr userData);
        void            DrawEntity(EditorEntity* editorEntity);

        String          m_searchEntity{};
        String          m_NameCache{};

        static void     AddEntity(VoidPtr userData);
        static void     AddEntityFromAsset(VoidPtr userData);
        static void     AddComponent(VoidPtr userData);
        static void     RenameEntity(VoidPtr userData);
        static void     DuplicateEntity(VoidPtr userData);
        static void     DeleteEntity(VoidPtr userData);

        static MenuItemContext s_menuItemContext;
    };
}
