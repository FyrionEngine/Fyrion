#pragma once

#include "Lib/imgui.h"
#include "Fyrion/Common.hpp"
#include "Fyrion/Platform/PlatformTypes.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/IO/InputTypes.hpp"

using namespace Fyrion;

namespace Fyrion
{
    class TypeHandler;
    class FieldHandler;
}

enum ImGuiContentTableFlags_
{
    ImGuiContentTableFlags_None             = 0,
    ImGuiContentTableFlags_SelectionNoFocus = 1 << 0,
};

enum ImGuiDrawTypeFlags_
{
    ImGuiDrawTypeFlags_None     = 0,
    ImGuiDrawTypeFlags_ReadOnly = 1 << 0,
};

typedef u32 ImGuiContentTableFlags;
typedef u32 ImGuiDrawTypeFlags;

namespace ImGui
{

    struct DrawTypeDesc;
    typedef void (*DrawTypeCallbackFn)(DrawTypeDesc& desc, ConstPtr newValue);
    //typedef void (*OnCreateSubobjectFn)(DrawTypeDesc& desc, RID newSubobject);

    struct ContentItemDesc
    {
        u32             ItemId{};
        const char*     PreLabel{};
        const char*     Label{};
        Texture         Texture{};
        bool            CanRename{};
        bool            ShowDetails{};
        const char*     DetailsDesc{};
        u32*            Color{};
        bool            DefaultSelected{};
        const char*     AcceptPayload{};
        const char*     SetPayload{};
        const char*     TooltipText{};
    };

    struct DrawTypeDesc
    {
        usize               itemId{};
       // RID                 rid;
        TypeHandler*        typeHandler{};
        ConstPtr            instance{};
        ImGuiDrawTypeFlags  flags{};
        VoidPtr             userData{};
        DrawTypeCallbackFn  callback{};
        //OnCreateSubobjectFn onCreateSubobject{};
    };

    struct DrawTypeContent
    {
        DrawTypeDesc  desc{};
        VoidPtr       instance{};
        u64           lastFrameUsage{};
        bool          readOnly{};
        bool          hasChanged{};
        bool          showResourceSelection{};
        bool          tableRender{};
        TypeID        resourceTypeSelection{};
        FieldHandler* fieldShowSelection{};
        Array<TypeID> graphOutputs{};
    };

    typedef void (*FieldRendererFn)(DrawTypeContent* context, FieldHandler* fieldHandler, VoidPtr value, bool* hasChanged);

    struct StyleColor
    {
        StyleColor(const StyleColor&) = delete;
        StyleColor operator=(const StyleColor&) = delete;

        template<typename T>
        StyleColor(ImGuiCol colorId, T colour)
        { ImGui::PushStyleColor(colorId, colour); }

        virtual ~StyleColor()
        {
            ImGui::PopStyleColor();
        }
    };

    struct StyleVar
    {
        StyleVar(const StyleVar&) = delete;
        StyleVar operator=(const StyleVar&) = delete;

        template<typename T>
        StyleVar(ImGuiStyleVar styleVar, T value)
        {
            ImGui::PushStyleVar(styleVar, value);
        }

        virtual ~StyleVar()
        {
            ImGui::PopStyleVar();
        }
    };

    FY_API void CreateDockSpace(ImGuiID dockSpaceId);
    FY_API bool Begin(u32 id, const char* name, bool* pOpen, ImGuiWindowFlags flags = 0);
    FY_API void DockBuilderReset(ImGuiID dockSpaceId);
    FY_API void DockBuilderDockWindow(ImGuiID windowId, ImGuiID nodeId);
    FY_API bool InputText(u32 idx, Fyrion::String& string, ImGuiInputTextFlags flags = 0);
    FY_API bool SearchInputText(ImGuiID idx, Fyrion::String& string, ImGuiInputTextFlags flags = 0);
    FY_API void BeginTreeNode();
    FY_API void EndTreeNode();
    FY_API bool TreeNode(u32 id, const char* label, ImGuiTreeNodeFlags flags = 0);
    FY_API bool TreeLeaf(u32 id, const char* label, ImGuiTreeNodeFlags flags = 0);
    FY_API void DrawImage(Texture texture, const Rect& rect, const ImVec4& tintCol = ImVec4(1, 1, 1, 1));

    FY_API bool BeginPopupMenu(const char* str, ImGuiWindowFlags popupFlags = 0, bool setSize = true);
    FY_API void EndPopupMenu(bool closePopup = true);
    FY_API bool SelectionButton(const char* label, bool selected, const ImVec2& sizeArg = ImVec2(0, 0));
    FY_API bool BorderedButton(const char* label, const ImVec2& size = ImVec2(0, 0));
    FY_API bool CollapsingHeaderProps(i32 id, const char* label, bool* buttonClicked);

    FY_API ImU32 TextToColor(const char* str);

    //content table
    FY_API bool       BeginContentTable(u32 id, f32 thumbnailSize, ImGuiContentTableFlags flags = 0);
    FY_API bool       DrawContentItem(const ContentItemDesc& contentItemDesc);
    FY_API void       CancelRenameContentSelected(ImGuiID id);
    FY_API bool       ContentItemHovered(ImGuiID itemId);
    FY_API bool       ContentItemSelected(ImGuiID itemId);
    FY_API void       SelectContentItem(ImGuiID itemId, ImGuiID tableId);
    FY_API bool       ContentItemFocused(ImGuiID itemId);
    FY_API void       RenameContentSelected(ImGuiID id);
    FY_API void       CancelRenameContentSelected(ImGuiID id);
    FY_API bool       ContentItemBeginPayload(ImGuiID itemId);
    FY_API bool       ContentItemAcceptPayload(ImGuiID itemId);
    FY_API bool       ContentItemRenamed(ImGuiID itemId);
    FY_API bool       RenamingSelected(ImGuiID itemId);
    FY_API StringView ContentRenameString();
    FY_API void       EndContentTable();

    FY_API void AddFieldRenderer(TypeID typeId, FieldRendererFn fieldRendererFn);
    FY_API void DrawType(const DrawTypeDesc& drawTypeDesc);

    FY_API ImGuiKey GetImGuiKey(Key key);

    //frame controlling
    void Init(Fyrion::Window window, Fyrion::Swapchain swapchain);
    void BeginFrame(Fyrion::Window window, Fyrion::f64 deltaTime);
    void Render(Fyrion::RenderCommands& renderCommands);
    void ImGuiShutdown();
}