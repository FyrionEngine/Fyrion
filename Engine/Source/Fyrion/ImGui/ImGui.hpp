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
    class Asset;
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
    typedef void (*DrawTypeCallbackFn)(DrawTypeDesc& desc, VoidPtr newValue);
    typedef void (*FnAssetSelectorCallback)(VoidPtr userData, Asset* asset);

    struct DrawTypeDesc
    {
        usize              itemId{};
        TypeHandler*       typeHandler{};
        VoidPtr            instance{};
        ImGuiDrawTypeFlags flags{};
        VoidPtr            userData{};
        DrawTypeCallbackFn callback{};
    };

    struct DrawTypeContent
    {
        DrawTypeDesc  desc{};
        VoidPtr       instance{};
        u64           lastFrameUsage{};
        bool          readOnly{};
        bool          hasChanged{};
        bool          tableRender{};
        Array<TypeID> graphOutputs{};
        u32           idCount{};
        FieldHandler* activeFieldHandler{};
        u32           editingId = U32_MAX;

        u32 ReserveID()
        {
            idCount += 10;
            return idCount;
        }
    };

    struct ContentItemDesc
    {
        usize      id;
        StringView label;
        Texture    texture;
        bool       selected;
        f32        thumbnailScale;
        bool       renameItem;
    };

    struct ContentItemState
    {
        bool   renameFinish;
        String newName;
        bool   hovered;
        bool   clicked;
        bool   doubleClicked;
    };

    typedef bool (*FieldRendererFn)(DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged);

    struct StyleColor
    {
        StyleColor(const StyleColor&) = delete;
        StyleColor operator=(const StyleColor&) = delete;

        template <typename T>
        StyleColor(ImGuiCol colorId, T colour)
        {
            ImGui::PushStyleColor(colorId, colour);
        }

        virtual ~StyleColor()
        {
            ImGui::PopStyleColor();
        }
    };

    struct StyleVar
    {
        StyleVar(const StyleVar&) = delete;
        StyleVar operator=(const StyleVar&) = delete;

        template <typename T>
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
    FY_API bool BeginFullscreen(u32 id, bool* pOpen = nullptr, ImGuiWindowFlags flags = 0);
    FY_API void DockBuilderReset(ImGuiID dockSpaceId);
    FY_API void DockBuilderDockWindow(ImGuiID windowId, ImGuiID nodeId);
    FY_API bool InputText(u32 idx, Fyrion::String& string, ImGuiInputTextFlags flags = 0);
    FY_API bool SearchInputText(ImGuiID idx, Fyrion::String& string, ImGuiInputTextFlags flags = 0);
    FY_API void BeginTreeNode();
    FY_API void EndTreeNode();
    FY_API bool TreeNode(u32 id, const char* label, ImGuiTreeNodeFlags flags = 0);
    FY_API bool TreeLeaf(u32 id, const char* label, ImGuiTreeNodeFlags flags = 0);
    FY_API void TextureItem(Texture       texture, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1),
                            const ImVec4& border_col = ImVec4(0, 0, 0, 0));
    FY_API void DrawTexture(Texture texture, const Rect& rect, const ImVec4& tintCol = ImVec4(1, 1, 1, 1));
    FY_API bool BeginPopupMenu(const char* str, ImGuiWindowFlags popupFlags = 0, bool setSize = true);
    FY_API void EndPopupMenu(bool closePopup = true);
    FY_API bool SelectionButton(const char* label, bool selected, const ImVec2& sizeArg = ImVec2(0, 0));
    FY_API bool BorderedButton(const char* label, const ImVec2& size = ImVec2(0, 0));
    FY_API bool CollapsingHeaderProps(i32 id, const char* label, bool* buttonClicked);
    FY_API void DummyRect(ImVec2 min, ImVec2 max);

    FY_API ImU32 TextToColor(const char* str);

    FY_API bool             BeginContentTable(const char* id, f32 thumbnailScale);
    FY_API ContentItemState ContentItem(const ContentItemDesc& contentItemDesc);
    FY_API void             EndContentTable();

    FY_API void ShowAssetSelector(TypeID assetId, VoidPtr userData, FnAssetSelectorCallback callback);

    FY_API void                  AddFieldRenderer(FieldRendererFn fieldRendererFn);
    FY_API Span<FieldRendererFn> GetFieldRenderers();
    FY_API void                  DrawType(const DrawTypeDesc& drawTypeDesc);
    FY_API void                  ClearDrawData(VoidPtr ptr, bool clearActiveId = true);

    FY_API ImGuiKey GetImGuiKey(Key key);

    //frame controlling
    void Init(Fyrion::Window window, Fyrion::Swapchain swapchain);
    void BeginFrame(Fyrion::Window window, Fyrion::f64 deltaTime);
    void Render(Fyrion::RenderCommands& renderCommands);
    void ImGuiShutdown();
}
