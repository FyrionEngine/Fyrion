#pragma once

#include "imgui.h"
#include "Fyrion/Common.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"

using namespace Fyrion;

namespace ImGui
{
    struct DrawTypeDesc;

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

    typedef void (*DrawTypeCallbackFn)(DrawTypeDesc& desc, VoidPtr newValue);

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
        ImVec2 screenStartPos;
        ImVec2 size;
    };

    typedef void (*FnAssetSelectorCallback)(VoidPtr userData, AssetFile* assetFile);
    typedef bool (*FieldRendererFn)(DrawTypeContent* context, const TypeInfo& typeInfo, VoidPtr value, bool* hasChanged);

    FY_API void                  ShowAssetSelector(TypeID assetId, VoidPtr userData, FnAssetSelectorCallback callback);
    FY_API void                  AddFieldRenderer(FieldRendererFn fieldRendererFn);
    FY_API Span<FieldRendererFn> GetFieldRenderers();
    FY_API void                  DrawType(const DrawTypeDesc& desc);
    FY_API void                  ClearDrawData(VoidPtr ptr, bool clearActiveId = true);

    FY_API bool             BeginContentTable(const char* id, f32 thumbnailScale);
    FY_API ContentItemState ContentItem(const ContentItemDesc& contentItemDesc);
    FY_API void             EndContentTable();

    FY_API void TextWithLabel(StringView label, StringView text);
}
