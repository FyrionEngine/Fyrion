
#include "MenuItem.hpp"
#include "Fyrion/ImGui/ImGui.hpp"

namespace Fyrion
{

    void MenuItemContext::AddMenuItem(const MenuItemCreation& menuItem)
    {
        Array<String> items = {};
        Split(StringView{menuItem.itemName}, StringView{"/"}, [&](const StringView& item)
        {
            items.EmplaceBack(item);
        });

        if (items.Empty())
        {
            items.EmplaceBack(menuItem.itemName);
        }

        MenuItemContext* parent = nullptr;
        MenuItemContext* storage = this;

        for(const String& item : items)
        {
            auto it = storage->m_menuItemsMap.Find(item);
            if (it == storage->m_menuItemsMap.end())
            {
                it = storage->m_menuItemsMap.Insert(item, MakeShared<MenuItemContext>()).first;
                it->second->m_label = item;
                storage->m_children.EmplaceBack(it->second.Get());
            }
            parent  = storage;
            storage = it->second.Get();
        }

        if (storage && parent)
        {
            storage->m_action = menuItem.action;
            storage->m_enable = menuItem.enable;
            storage->m_itemShortcut = menuItem.itemShortcut;

            if (!menuItem.icon.Empty())
            {
                String label = storage->m_label;
                storage->m_label = menuItem.icon;
                storage->m_label += " ";
                storage->m_label += label;
            }
            storage->m_priority = menuItem.priority;

            Sort(parent->m_children.begin(), parent->m_children.end(), [](MenuItemContext* a, MenuItemContext* b)
            {
                return a->m_priority < b->m_priority;
            });
        }
    }

    void MenuItemContext::DrawMenuItemChildren(MenuItemContext* context, VoidPtr userData)
    {
        bool enabled = true;
        if (context->m_enable)
        {
            enabled =  context->m_enable(userData);
        }

        bool isItem = context->m_children.Empty();
        if (isItem)
        {
            BufferString<64> shortcut{};

            if (context->m_itemShortcut.ctrl)
            {
                shortcut += "Ctrl+";
            }

            if (context->m_itemShortcut.alt)
            {
                shortcut += "Alt+";
            }

            if (context->m_itemShortcut.shift)
            {
                shortcut += "Shift+";
            }

            if (context->m_itemShortcut.presKey != Key::None)
            {
                shortcut += ImGui::GetKeyName(ImGui::GetImGuiKey(context->m_itemShortcut.presKey));
            }

            if (ImGui::MenuItem(context->m_label.CStr(), shortcut.begin(), false, enabled))
            {
                if (context->m_action)
                {
                    context->m_action(userData);
                }
            }
        }
        else
        {
            if (ImGui::BeginMenu(context->m_label.CStr(), enabled))
            {
                i32 lastPriority = context->m_children[0]->m_priority;
                for (MenuItemContext* child: context->m_children)
                {
                    if ((lastPriority + 50) < child->m_priority)
                    {
                        ImGui::Separator();
                    }
                    DrawMenuItemChildren(child, userData);
                    lastPriority = child->m_priority;
                }
                ImGui::EndMenu();
            }
        }
    }

    void MenuItemContext::Draw(VoidPtr userData)
    {
        i32 lastPriority = 0;
        for (MenuItemContext* menuItemStorage: this->m_children)
        {
            if (lastPriority + 50 < menuItemStorage->m_priority)
            {
                ImGui::Separator();
            }
            DrawMenuItemChildren(menuItemStorage, userData);
            lastPriority = menuItemStorage->m_priority;
        }
    }

    bool MenuItemContext::ExecuteHotKeys(MenuItemContext* context, VoidPtr userData)
    {
        bool executed = false;
        if (context->m_itemShortcut.presKey != Key::None && context->m_action)
        {
            bool ctrlHolding = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftCtrl)) || ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightCtrl));
            bool shiftHolding = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftShift)) || ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightShift));
            bool altHolding = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftAlt)) || ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightAlt));

            bool ctrlState = (context->m_itemShortcut.ctrl && ctrlHolding) || !context->m_itemShortcut.ctrl;
            bool shitState = (context->m_itemShortcut.shift && shiftHolding) || !context->m_itemShortcut.shift;
            bool altState = (context->m_itemShortcut.alt && altHolding) || !context->m_itemShortcut.alt;

            bool keyPressed = ImGui::IsKeyPressed(ImGui::GetImGuiKey(context->m_itemShortcut.presKey));
            if (keyPressed && ctrlState && shitState && altState)
            {
                bool enabled = true;
                if (context->m_enable)
                {
                    enabled = context->m_enable(userData);
                }
                if (enabled)
                {
                    context->m_action(userData);
                    executed = true;
                }
            }
        }

        for (MenuItemContext* menuItemStorage: context->m_children)
        {
            if (ExecuteHotKeys(menuItemStorage, userData))
            {
                executed = true;
            }
        }

        return executed;
    }

    bool MenuItemContext::ExecuteHotKeys(VoidPtr userData)
    {
        return ExecuteHotKeys(this, userData);
    }
}