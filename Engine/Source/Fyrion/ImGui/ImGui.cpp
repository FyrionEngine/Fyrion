#include "ImGui.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include "Fyrion/IO/InputTypes.hpp"
#include "ImGuiPlatform.hpp"
#include "Fyrion/Graphics/Device/RenderDevice.hpp"
#include "Fyrion/Resource/ResourceObject.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Assets/AssetTypes.hpp"
#include "IconsFontAwesome6.h"
#include "Fyrion/ImGui/Lib/imgui_internal.h"
#include "Fyrion/ImGui/Lib/ImGuizmo.h"

using namespace Fyrion;

namespace Fyrion
{
    RenderDevice& GetRenderDevice();
}

namespace ImGui
{

    struct ContentTable
    {
        f32 thumbnailSize{};
        u32 selectedItem{};
        u32 hoveredItem{};
        bool renamingSelected{};
        u32 renamingId{};
        String renamingStringCache{};
        u32 renamedItem{};
        u32 renamedCancelled{};
        bool renamingFocus{};
        bool selectionNoFocus{};
        u32 acceptPayloadItem{U32_MAX};
        u32 beginPayloadItem{U32_MAX};
    };

    namespace
    {
        f32 scaleFactor = 1.0;
        ImGuiKey keys[static_cast<u32>(Key::MAX)];

        HashMap<ImGuiID, ContentTable> contentTables{};
        ImGuiID currentContentTable{U32_MAX};
    }

    struct InputTextCallback_UserData
    {
        String& str;
    };

    static int InputTextCallback(ImGuiInputTextCallbackData* data)
    {
        auto userData = (InputTextCallback_UserData*) data->UserData;
        if (data->BufTextLen >= (userData->str.Capacity() - 1))
        {
            auto new_size = userData->str.Capacity() + (userData->str.Capacity() * 2) / 3;
            userData->str.Reserve(new_size);
            data->Buf = userData->str.begin();
        }
        userData->str.SetSize(data->BufTextLen);
        return 0;
    }

    void CreateDockSpace(ImGuiID dockSpaceId)
    {
        static ImGuiDockNodeFlags dockNodeFlags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        auto viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(20, 20, 23, 255));
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        dockNodeFlags |= ImGuiDockNodeFlags_NoWindowMenuButton;

        ImGui::Begin("DockSpace", 0, windowFlags);
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(1);

        ImGuiStyle& style = ImGui::GetStyle();
        ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, viewport->WorkSize.y - 40 * style.ScaleFactor), dockNodeFlags);

//		auto* drawList = ImGui::GetWindowDrawList();
//		drawList->AddLine(ImVec2(0.0f, viewport->WorkSize.y - 30), ImVec2(viewport->WorkSize.x, viewport->WorkSize.y), IM_COL32(0, 0, 0, 255), 1.f * style.ScaleFactor);
    }

    bool Begin(u32 id, const char* name, bool* pOpen, ImGuiWindowFlags flags)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImGui::SetNextWindowSize({800 * style.ScaleFactor, 400 * style.ScaleFactor}, ImGuiCond_Once);

        char str[100];
        sprintf(str, "%s###%d", name, id);
        bool open = ImGui::Begin(str, pOpen, flags);
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
        {
            //hoveredWindowId = id;
        }
        return open;
    }

    void DockBuilderReset(ImGuiID dockSpaceId)
    {
        auto viewport = ImGui::GetMainViewport();
        ImGui::DockBuilderRemoveNode(dockSpaceId);
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport->WorkSize);
    }

    void DockBuilderDockWindow(ImGuiID windowId, ImGuiID nodeId)
    {
        char str[20];
        sprintf(str, "###%d", windowId);
        ImGui::DockBuilderDockWindow(str, nodeId);
    }

    bool InputText(u32 idx, Fyrion::String& string, ImGuiInputTextFlags flags)
    {
        char str[100];
        sprintf(str, "###txtid%d", idx);

        InputTextCallback_UserData user_data{string};
        flags |= ImGuiInputTextFlags_CallbackResize;

        auto ret = ImGui::InputText(str, string.begin(), string.Capacity() + 1, flags, InputTextCallback, &user_data);

        auto& imguiContext = *ImGui::GetCurrentContext();

        auto& rect = imguiContext.LastItemData.Rect;
        auto* drawList = ImGui::GetWindowDrawList();

        if (ImGui::IsItemFocused())
        {
            drawList->AddRect(rect.Min, ImVec2(rect.Max.x - ImGui::GetStyle().ScaleFactor, rect.Max.y), IM_COL32(66, 140, 199, 255), ImGui::GetStyle().FrameRounding, 0,
                1 * ImGui::GetStyle().ScaleFactor);
        }

        return ret;
    }

    bool SearchInputText(ImGuiID idx, Fyrion::String& string, ImGuiInputTextFlags flags)
    {
        const auto searching = !string.Empty();

        auto& style = ImGui::GetStyle();
        const float newPadding = 28.0f * ImGui::GetStyle().ScaleFactor;
        auto& context = *ImGui::GetCurrentContext();
        auto* drawList = ImGui::GetWindowDrawList();
        auto& rect = context.LastItemData.Rect;

        ImGui::StyleVar styleVar{ImGuiStyleVar_FramePadding, ImVec2(newPadding, style.FramePadding.y)};

        auto modified = InputText(idx, string, flags);

        if (!searching)
        {
            drawList->AddText(ImVec2(rect.Min.x + newPadding, rect.Min.y + style.FramePadding.y), ImGui::GetColorU32(ImGuiCol_TextDisabled), "Search");
        }

        drawList->AddText(ImVec2(rect.Min.x + style.ItemInnerSpacing.x, rect.Min.y + style.FramePadding.y), ImGui::GetColorU32(ImGuiCol_Text), ICON_FA_MAGNIFYING_GLASS);

        return modified;
    }

    void BeginTreeNode()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.26f, 0.59f, 0.98f, 0.67f)); //TODO - get from config (selected item)
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.26f, 0.59f, 0.98f, 0.67f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.0, 0.0f, 0.0f, 0.0f));
    }

    void EndTreeNode()
    {
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
    }

    bool TreeNode(u32 id, const char* label, ImGuiTreeNodeFlags flags)
    {
        flags |= ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_SpanFullWidth;

        return ImGui::TreeNodeEx((void*) (usize) id, flags, "%s", label);
    }

    bool TreeLeaf(u32 id, const char* label, ImGuiTreeNodeFlags flags)
    {
        flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf |
            ImGuiTreeNodeFlags_SpanFullWidth |
            ImGuiTreeNodeFlags_NoTreePushOnOpen;
        return ImGui::TreeNodeEx((void*) (usize) id, flags, "%s", label);
    }

    void DrawImage(Texture texture, const Rect& rect, const ImVec4& tintCol)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();

//      TODO - keep the aspect ratio of the texture?
//      auto& container = *static_cast<TextureContainer*>(texture);
//      auto ratio = container.size.width / container.size.height;
//      ImVec2(rect.width / , rect.height / ),

//        drawList->AddImage(
//            RenderDevice::GetTextureHandler(texture),
//            ImVec2(rect.X, rect.Y),
//            ImVec2(rect.Width, rect.Height),
//            ImVec2(0, 0),
//            ImVec2(1, 1),
//            ImGui::ColorConvertFloat4ToU32(tintCol));
    }

    bool BeginPopupMenu(const char* str, ImGuiWindowFlags popupFlags, bool setSize)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{6 * style.ScaleFactor, 4 * style.ScaleFactor});
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{1, 1});

        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.26f, 0.59f, 0.98f, 0.67f)); //TODO - get from config (selected item)
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.46f, 0.49f, 0.50f, 0.67f));
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.46f, 0.49f, 0.50f, 0.67f));

        if (setSize)
        {
            ImGui::SetNextWindowSize(ImVec2{300, 0,}, ImGuiCond_Once);
        }
        bool res = ImGui::BeginPopup(str, popupFlags);
        return res;
    }

    void EndPopupMenu(bool closePopup)
    {
        if (closePopup)
        {
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
    }

    bool SelectionButton(const char* label, bool selected, const ImVec2& sizeArg)
    {
        //TODO make select
        return Button(label, sizeArg);
    }


    bool BorderedButton(const char* label, const ImVec2& size)
    {
        ImGui::StyleColor border(ImGuiCol_Border,  ImVec4(0.46f, 0.49f, 0.50f, 0.67f));
        return ImGui::Button(label, size);
    }


    ImU32 TextToColor(const char* str)
    {
        auto color = ImHashStr(str, strlen(str));
        auto vecColor = ImGui::ColorConvertU32ToFloat4(color);
        return IM_COL32(vecColor.x * 255, vecColor.y * 255, vecColor.z * 255, 255);
    }

    ContentTable& CurrentTable()
    {
        return contentTables[currentContentTable];
    }

    ContentTable& CurrentTable(ImGuiID id)
    {
        return contentTables[id];
    }

    bool BeginContentTable(u32 id, f32 ThumbnailSize, ImGuiContentTableFlags flags)
    {

        auto& contentTable = contentTables[id];
        contentTable.thumbnailSize = ThumbnailSize;
        currentContentTable = id;
        contentTable.hoveredItem = U32_MAX;
        contentTable.renamedItem = U32_MAX;
        contentTable.renamedCancelled = U32_MAX;
        contentTable.selectionNoFocus = (flags & ImGuiContentTableFlags_SelectionNoFocus);

        if (contentTable.renamingSelected && contentTable.renamingId != contentTable.selectedItem)
        {
            CancelRenameContentSelected(id);
        }

        static ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedSame;
        auto cellPadding = ImGui::GetStyle().CellPadding;

        auto totalThumbnailSize = ThumbnailSize + cellPadding.x;

        u32 columns = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x) / totalThumbnailSize;
        columns = Math::Max(columns, 1u);
        bool tableRes = ImGui::BeginTable("ContentTable", columns, tableFlags);

        if (tableRes)
        {
            for (int i = 0; i < columns; ++i)
            {

                char buffer[20]{};
                StringConverter<i32>::ToString(buffer, 0, i);
                ImGui::TableSetupColumn(buffer, ImGuiTableColumnFlags_WidthFixed, ThumbnailSize);
            }
        }
        return tableRes;
    }

    bool DrawContentItem(const ContentItemDesc& contentItemDesc)
    {
        FY_ASSERT(currentContentTable != U32_MAX, "missing BeginContentTable");

        auto& contentTable = contentTables[currentContentTable];
        auto ThumbnailSize = contentTable.thumbnailSize;
        contentTable.acceptPayloadItem = U32_MAX;
        contentTable.beginPayloadItem = U32_MAX;

        ImGui::TableNextColumn();

        auto* drawList = ImGui::GetWindowDrawList();
        auto cursorPos = ImGui::GetCursorScreenPos();
        cursorPos.x += 2;
        auto table = ImGui::GetCurrentTable();
        auto width = table->Columns.Data->WidthGiven;
        auto contentInfoSize = ThumbnailSize + (ThumbnailSize / 2);
        auto contentFullSize = ImVec2(width, contentInfoSize);
        auto bottomRight = ImVec2(cursorPos.x + width, cursorPos.y + contentInfoSize);
        auto bottomIcon = ImVec2(cursorPos.x + width, cursorPos.y + ThumbnailSize);

//        auto topIcon = ImVec2(cursorPos.x + width - 1, cursorPos.y + 13 * ImGui::GetStyle().ScaleFactor);

        ImGui::ItemAdd(ImRect(cursorPos, bottomRight), contentItemDesc.ItemId);
        ImGui::PushID(contentItemDesc.ItemId);
        ImGui::BeginGroup();

        bool windowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        bool isHovered = ImGui::IsItemHovered();
        bool isSelected = contentTable.selectedItem == contentItemDesc.ItemId;
        i32 mouseCount = ImGui::GetMouseClickedCount(ImGuiMouseButton_Left);
        bool isDoubleClicked = mouseCount >= 2 && (mouseCount % 2) == 0 && isHovered;
        bool isEnter =
            (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_KeypadEnter))) && ContentItemFocused(contentItemDesc.ItemId) &&
                !contentTable.renamingSelected && windowHovered;
        bool isClicked = (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && isHovered;

        auto scaleFactor = ImGui::GetStyle().ScaleFactor;
        auto separatorLineSize = 2 * scaleFactor;

        if (isHovered)
        {
            contentTable.hoveredItem = contentItemDesc.ItemId;
        }

        if (contentItemDesc.TooltipText && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
        {
            ImGui::TextUnformatted(contentItemDesc.TooltipText);
            ImGui::EndTooltip();
        }

        if (contentItemDesc.ShowDetails)
        {
            //shadow
            drawList->AddRect(ImVec2(cursorPos.x + 1.5 * ImGui::GetStyle().ScaleFactor, cursorPos.y + 1.5 * ImGui::GetStyle().ScaleFactor),
                ImVec2(bottomRight.x + 1.5 * ImGui::GetStyle().ScaleFactor, bottomRight.y + 1.5 * ImGui::GetStyle().ScaleFactor),
                IM_COL32(2, 4, 6, 255),
                6.0f, 0, 3);

            drawList->AddRectFilled(cursorPos, bottomIcon, IM_COL32(20, 21, 23, 255), 6, ImDrawFlags_RoundCornersTop);

            //bottom color
            drawList->AddRectFilled(ImVec2(cursorPos.x, cursorPos.y + ThumbnailSize),
                ImVec2(bottomRight.x, bottomRight.y),
                IM_COL32(32, 33, 35, 255), 6, ImDrawFlags_RoundCornersBottom);

        }
        else if (isHovered || isSelected)
        {

            //shadow
            drawList->AddRect(ImVec2(cursorPos.x + 1, cursorPos.y + 1), ImVec2(bottomRight.x + 1 * ImGui::GetStyle().ScaleFactor, bottomRight.y + 1 * ImGui::GetStyle().ScaleFactor),
                IM_COL32(2, 4, 6, 255),
                6.0f, 0, 3);

            drawList->AddRectFilled(cursorPos, bottomRight,
                IM_COL32(40, 41, 43, 255),
                6.0f, 0);
        }

        ImVec2 oldCursor = ImGui::GetCursorPos();
        char str[50];
        sprintf(str, "###invisibebutton_%d", contentItemDesc.ItemId);

        ImGui::SetCursorPos(ImVec2(oldCursor.x + 4, oldCursor.y));
        ImGui::InvisibleButton(str, ImVec2(contentFullSize.x - 4, contentFullSize.y));

        if (contentItemDesc.AcceptPayload != nullptr && ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(contentItemDesc.AcceptPayload))
            {
                contentTable.acceptPayloadItem = contentItemDesc.ItemId;
            }
            ImGui::EndDragDropTarget();
        }

        if (contentItemDesc.SetPayload != nullptr && !isDoubleClicked && ImGui::BeginDragDropSource())
        {
            contentTable.beginPayloadItem = contentItemDesc.ItemId;

            ImGui::SetDragDropPayload(contentItemDesc.SetPayload, nullptr, 0);
            ImGui::Text("%s", contentItemDesc.Label);
            ImGui::EndDragDropSource();
        }

        ImGui::SetCursorPos(oldCursor);

        ImGui::ItemSize(ImVec2{width, ThumbnailSize});
        //drawImage(contentItemDesc.texture, {cursorPos.x, cursorPos.y, cursorPos.x + width, cursorPos.y + ThumbnailSize});
        if (contentItemDesc.Texture)
        {
            DrawImage(contentItemDesc.Texture, {(i32) cursorPos.x,
                                                (i32) cursorPos.y,
                                                (u32) (cursorPos.x + width),
                                                (u32) (cursorPos.y + ThumbnailSize)});
        }

        if (contentItemDesc.ShowDetails)
        {
            drawList->AddRectFilled(ImVec2(cursorPos.x, cursorPos.y - separatorLineSize + ThumbnailSize),
                ImVec2(bottomRight.x + 1, cursorPos.y + ThumbnailSize),
                contentItemDesc.Color != nullptr ? *contentItemDesc.Color : TextToColor(contentItemDesc.DetailsDesc), 0, 0);

            if (isHovered && !isSelected)
            {
                drawList->AddRect(ImVec2(cursorPos.x - 1, cursorPos.y - 1), ImVec2(bottomRight.x + 1, bottomRight.y + 1), IM_COL32(90, 90, 90, 255), 6, 0, 1);
            }
        }

        if (isSelected)
        {
            drawList->AddRect(ImVec2(cursorPos.x - 1, cursorPos.y - 1), ImVec2(bottomRight.x + 1, bottomRight.y + 1),
                ImGui::ColorConvertFloat4ToU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f)),
                6.0f, 0, 1);

//			drawList->AddRect(ImVec2(cursorPos.x - 1, cursorPos.y - 1), ImVec2(bottomRight.x + 1, bottomRight.y + 1),
//				IM_COL32(213, 124, 22, 255),
//				6.0f, 0, 1);
        }

        //separator size
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + separatorLineSize);

        ImGui::BeginVertical(contentItemDesc.ItemId + 1, ImVec2(width, contentInfoSize - ThumbnailSize), 0.2);
        {
            ImGui::BeginHorizontal(contentItemDesc.ItemId + 2, ImVec2(width, 0.0f));

            ImGui::Spring();
            {

                auto cursor = ImGui::GetCursorScreenPos();

                auto textSize = ImGui::CalcTextSize(contentItemDesc.Label);

                if (contentTable.renamingSelected && isSelected)
                {

                    auto magicNumber = 8.f;

                    ImGui::SetNextItemWidth(width - magicNumber);
                    ImGui::SetCursorScreenPos(ImVec2{cursor.x - magicNumber, cursor.y});

                    if (!contentTable.renamingFocus)
                    {
                        contentTable.renamingStringCache = contentItemDesc.Label;
                        ImGui::SetKeyboardFocusHere();
                    }

                    ImGui::StyleColor frameColor(ImGuiCol_FrameBg, IM_COL32(52, 53, 55, 255));
                    InputText(40101, contentTable.renamingStringCache);

                    //defer renamingFocus change, because InputText is not active in the first frame, canceling rename
                    if (!ImGui::IsItemActive() && contentTable.renamingFocus)
                    {
                        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                        {
                            contentTable.renamedCancelled = contentItemDesc.ItemId;
                        }
                        else
                        {
                            contentTable.renamedItem = contentItemDesc.ItemId;
                        }

                        contentTable.renamingSelected = false;
                        contentTable.renamingFocus = false;
                        ImGui::SetFocusID(Math::Max(contentItemDesc.ItemId, 1u), ImGui::GetCurrentWindow());

                    }
                    else if (!contentTable.renamingFocus)
                    {
                        contentTable.renamingFocus = true;
                    }
                }
                else
                {
                    auto rect = ImVec2{cursor.x + ThumbnailSize - 20, cursor.y + (textSize.y * 2)};

                    //TODO PushTextWrapPos + PushClipRect is messing with the CursorPos if the text is too large.
                    ImGui::PushClipRect(cursor, rect, true);

//                    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width - 15);

                    const float textWidth = Math::Min(textSize.x, width);

                    ImGui::SetNextItemWidth(textWidth);
                    ImGui::Text("%s%s", contentItemDesc.PreLabel != nullptr ? contentItemDesc.PreLabel : "", contentItemDesc.Label);

//                    ImGui::PopTextWrapPos();

                    ImGui::PopClipRect();
                }
            }

            ImGui::Spring();
            ImGui::EndHorizontal();

            ImGui::Spring();

            if (isClicked)
            {
                contentTable.selectedItem = contentItemDesc.ItemId;
                ImGui::SetFocusID(Math::Max(contentItemDesc.ItemId, 1u), ImGui::GetCurrentWindow());
            }

            if (contentItemDesc.ShowDetails)
            {
                ImGui::TextDisabled("%s", contentItemDesc.DetailsDesc);
                ImGui::Spring(0.4);
            }
            ImGui::EndVertical();
        }

        ImGui::EndGroup();
        ImGui::PopID();

        return (isDoubleClicked || isEnter) && !contentTable.renamingSelected;
    }

    void EndContentTable()
    {
        ImGui::EndTable();
        currentContentTable = U32_MAX;
    }

    void CancelRenameContentSelected(ImGuiID id)
    {
        ContentTable& contentTable = CurrentTable(id);
        contentTable.renamingSelected = false;
        contentTable.renamedItem = U32_MAX;
        contentTable.renamingId = U32_MAX;
        contentTable.renamingStringCache.Clear();
        contentTable.renamingSelected = false;
        contentTable.renamingFocus = false;
    }

    bool ContentItemHovered(ImGuiID itemId)
    {
        return CurrentTable().hoveredItem == itemId;
    }

    bool ContentItemSelected(ImGuiID itemId)
    {
        return CurrentTable().selectedItem == itemId;
    }

    void SelectContentItem(ImGuiID itemId, ImGuiID tableId)
    {
        if (tableId == U32_MAX)
        {
            CurrentTable().selectedItem = itemId;
        }
        else
        {
            CurrentTable(tableId).selectedItem = itemId;
        }
    }

    bool ContentItemFocused(ImGuiID itemId)
    {
        return ImGui::GetFocusID() == itemId;
    }

    void RenameContentSelected(ImGuiID id)
    {
        auto& contentTable = CurrentTable(id);
        if (contentTable.selectedItem)
        {
            contentTable.renamingSelected = true;
            contentTable.renamingId = contentTable.selectedItem;
        }
    }

    bool ContentItemRenamed(ImGuiID itemId)
    {
        return CurrentTable().renamedItem == itemId;
    }

    bool ContentItemBeginPayload(ImGuiID itemId)
    {
        return CurrentTable().beginPayloadItem == itemId;
    }

    bool ContentItemAcceptPayload(ImGuiID itemId)
    {
        return CurrentTable().acceptPayloadItem == itemId;
    }

    bool RenamingSelected(ImGuiID itemId)
    {
        auto& ct = CurrentTable(itemId);
        return ct.renamingSelected;
    }

    StringView ContentRenameString()
    {
        return CurrentTable().renamingStringCache;
    }

    void RegisterKeys()
    {
        keys[static_cast<u32>(Key::Space)] = ImGuiKey_Space;
        keys[static_cast<u32>(Key::Apostrophe)] = ImGuiKey_Apostrophe;
        keys[static_cast<u32>(Key::Comma)] = ImGuiKey_Comma;
        keys[static_cast<u32>(Key::Minus)] = ImGuiKey_Minus;
        keys[static_cast<u32>(Key::Period)] = ImGuiKey_Period;
        keys[static_cast<u32>(Key::Slash)] = ImGuiKey_Slash;
        keys[static_cast<u32>(Key::Num0)] = ImGuiKey_0;
        keys[static_cast<u32>(Key::Num1)] = ImGuiKey_1;
        keys[static_cast<u32>(Key::Num2)] = ImGuiKey_2;
        keys[static_cast<u32>(Key::Num3)] = ImGuiKey_3;
        keys[static_cast<u32>(Key::Num4)] = ImGuiKey_4;
        keys[static_cast<u32>(Key::Num5)] = ImGuiKey_5;
        keys[static_cast<u32>(Key::Num6)] = ImGuiKey_6;
        keys[static_cast<u32>(Key::Num7)] = ImGuiKey_7;
        keys[static_cast<u32>(Key::Num8)] = ImGuiKey_8;
        keys[static_cast<u32>(Key::Num9)] = ImGuiKey_9;
        keys[static_cast<u32>(Key::Semicolon)] = ImGuiKey_Semicolon;
        keys[static_cast<u32>(Key::Equal)] = ImGuiKey_Equal;
        keys[static_cast<u32>(Key::A)] = ImGuiKey_A;
        keys[static_cast<u32>(Key::B)] = ImGuiKey_B;
        keys[static_cast<u32>(Key::C)] = ImGuiKey_C;
        keys[static_cast<u32>(Key::D)] = ImGuiKey_D;
        keys[static_cast<u32>(Key::E)] = ImGuiKey_E;
        keys[static_cast<u32>(Key::F)] = ImGuiKey_F;
        keys[static_cast<u32>(Key::G)] = ImGuiKey_G;
        keys[static_cast<u32>(Key::H)] = ImGuiKey_H;
        keys[static_cast<u32>(Key::I)] = ImGuiKey_I;
        keys[static_cast<u32>(Key::J)] = ImGuiKey_J;
        keys[static_cast<u32>(Key::K)] = ImGuiKey_K;
        keys[static_cast<u32>(Key::L)] = ImGuiKey_L;
        keys[static_cast<u32>(Key::M)] = ImGuiKey_M;
        keys[static_cast<u32>(Key::N)] = ImGuiKey_N;
        keys[static_cast<u32>(Key::O)] = ImGuiKey_O;
        keys[static_cast<u32>(Key::P)] = ImGuiKey_P;
        keys[static_cast<u32>(Key::Q)] = ImGuiKey_Q;
        keys[static_cast<u32>(Key::R)] = ImGuiKey_R;
        keys[static_cast<u32>(Key::S)] = ImGuiKey_S;
        keys[static_cast<u32>(Key::T)] = ImGuiKey_T;
        keys[static_cast<u32>(Key::U)] = ImGuiKey_U;
        keys[static_cast<u32>(Key::V)] = ImGuiKey_V;
        keys[static_cast<u32>(Key::W)] = ImGuiKey_W;
        keys[static_cast<u32>(Key::X)] = ImGuiKey_X;
        keys[static_cast<u32>(Key::Y)] = ImGuiKey_Y;
        keys[static_cast<u32>(Key::Z)] = ImGuiKey_Z;
        keys[static_cast<u32>(Key::LeftBracket)] = ImGuiKey_LeftBracket;
        keys[static_cast<u32>(Key::Backslash)] = ImGuiKey_Backslash;
        keys[static_cast<u32>(Key::RightBracket)] = ImGuiKey_RightBracket;
        keys[static_cast<u32>(Key::GraveAccent)] = ImGuiKey_GraveAccent;
//		keys[static_cast<u32>(Key::World1)] = ImGuiKey_World1;
//		keys[static_cast<u32>(Key::World2)] = ImGuiKey_World2;
        keys[static_cast<u32>(Key::Escape)] = ImGuiKey_Escape;
        keys[static_cast<u32>(Key::Enter)] = ImGuiKey_Enter;
        keys[static_cast<u32>(Key::Tab)] = ImGuiKey_Tab;
        keys[static_cast<u32>(Key::Backspace)] = ImGuiKey_Backspace;
        keys[static_cast<u32>(Key::Insert)] = ImGuiKey_Insert;
        keys[static_cast<u32>(Key::Delete)] = ImGuiKey_Delete;
        keys[static_cast<u32>(Key::Right)] = ImGuiKey_RightArrow;
        keys[static_cast<u32>(Key::Left)] = ImGuiKey_LeftArrow;
        keys[static_cast<u32>(Key::Down)] = ImGuiKey_DownArrow;
        keys[static_cast<u32>(Key::Up)] = ImGuiKey_UpArrow;
        keys[static_cast<u32>(Key::PageUp)] = ImGuiKey_PageUp;
        keys[static_cast<u32>(Key::PageDown)] = ImGuiKey_PageDown;
        keys[static_cast<u32>(Key::Home)] = ImGuiKey_Home;
        keys[static_cast<u32>(Key::End)] = ImGuiKey_End;
        keys[static_cast<u32>(Key::CapsLock)] = ImGuiKey_CapsLock;
        keys[static_cast<u32>(Key::ScrollLock)] = ImGuiKey_ScrollLock;
        keys[static_cast<u32>(Key::NumLock)] = ImGuiKey_NumLock;
        keys[static_cast<u32>(Key::PrintScreen)] = ImGuiKey_PrintScreen;
        keys[static_cast<u32>(Key::Pause)] = ImGuiKey_Pause;
        keys[static_cast<u32>(Key::F1)] = ImGuiKey_F1;
        keys[static_cast<u32>(Key::F2)] = ImGuiKey_F2;
        keys[static_cast<u32>(Key::F3)] = ImGuiKey_F3;
        keys[static_cast<u32>(Key::F4)] = ImGuiKey_F4;
        keys[static_cast<u32>(Key::F5)] = ImGuiKey_F5;
        keys[static_cast<u32>(Key::F6)] = ImGuiKey_F6;
        keys[static_cast<u32>(Key::F7)] = ImGuiKey_F7;
        keys[static_cast<u32>(Key::F8)] = ImGuiKey_F8;
        keys[static_cast<u32>(Key::F9)] = ImGuiKey_F9;
        keys[static_cast<u32>(Key::F10)] = ImGuiKey_F10;
        keys[static_cast<u32>(Key::F11)] = ImGuiKey_F11;
        keys[static_cast<u32>(Key::F12)] = ImGuiKey_F12;
        keys[static_cast<u32>(Key::F13)] = ImGuiKey_F13;
        keys[static_cast<u32>(Key::F14)] = ImGuiKey_F14;
        keys[static_cast<u32>(Key::F15)] = ImGuiKey_F15;
        keys[static_cast<u32>(Key::F16)] = ImGuiKey_F16;
        keys[static_cast<u32>(Key::F17)] = ImGuiKey_F17;
        keys[static_cast<u32>(Key::F18)] = ImGuiKey_F18;
        keys[static_cast<u32>(Key::F19)] = ImGuiKey_F19;
        keys[static_cast<u32>(Key::F20)] = ImGuiKey_F20;
        keys[static_cast<u32>(Key::F21)] = ImGuiKey_F21;
        keys[static_cast<u32>(Key::F22)] = ImGuiKey_F22;
        keys[static_cast<u32>(Key::F23)] = ImGuiKey_F23;
        keys[static_cast<u32>(Key::F24)] = ImGuiKey_F24;
        keys[static_cast<u32>(Key::Keypad0)] = ImGuiKey_Keypad0;
        keys[static_cast<u32>(Key::Keypad1)] = ImGuiKey_Keypad1;
        keys[static_cast<u32>(Key::Keypad2)] = ImGuiKey_Keypad2;
        keys[static_cast<u32>(Key::Keypad3)] = ImGuiKey_Keypad3;
        keys[static_cast<u32>(Key::Keypad4)] = ImGuiKey_Keypad4;
        keys[static_cast<u32>(Key::Keypad5)] = ImGuiKey_Keypad5;
        keys[static_cast<u32>(Key::Keypad6)] = ImGuiKey_Keypad6;
        keys[static_cast<u32>(Key::Keypad7)] = ImGuiKey_Keypad7;
        keys[static_cast<u32>(Key::Keypad8)] = ImGuiKey_Keypad8;
        keys[static_cast<u32>(Key::Keypad9)] = ImGuiKey_Keypad9;
        keys[static_cast<u32>(Key::KeypadDecimal)] = ImGuiKey_KeypadDecimal;
        keys[static_cast<u32>(Key::KeypadDivide)] = ImGuiKey_KeypadDivide;
        keys[static_cast<u32>(Key::KeypadMultiply)] = ImGuiKey_KeypadMultiply;
        keys[static_cast<u32>(Key::KeypadSubtract)] = ImGuiKey_KeypadSubtract;
        keys[static_cast<u32>(Key::KeypadAdd)] = ImGuiKey_KeypadAdd;
        keys[static_cast<u32>(Key::KeypadEnter)] = ImGuiKey_KeypadEnter;
        keys[static_cast<u32>(Key::KeypadEqual)] = ImGuiKey_KeypadEqual;
        keys[static_cast<u32>(Key::LeftShift)] = ImGuiKey_LeftShift;
        keys[static_cast<u32>(Key::LeftCtrl)] = ImGuiKey_LeftCtrl;
        keys[static_cast<u32>(Key::LeftAlt)] = ImGuiKey_LeftAlt;
        keys[static_cast<u32>(Key::LeftSuper)] = ImGuiKey_LeftSuper;
        keys[static_cast<u32>(Key::RightShift)] = ImGuiKey_RightShift;
        keys[static_cast<u32>(Key::RightCtrl)] = ImGuiKey_RightCtrl;
        keys[static_cast<u32>(Key::RightAlt)] = ImGuiKey_RightAlt;
        keys[static_cast<u32>(Key::RightSuper)] = ImGuiKey_RightSuper;
        keys[static_cast<u32>(Key::Menu)] = ImGuiKey_Menu;
    }

    void ApplyDefaultStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        colors[ImGuiCol_Text] = ImVec4(0.71f, 0.72f, 0.71f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.12f, 0.13f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
        colors[ImGuiCol_Border] = ImVec4(0.01f, 0.01f, 0.02f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.11f, 0.12f, 0.13f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.59f, 0.60f, 0.59f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.59f, 0.60f, 0.59f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.12f);
        colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
        colors[ImGuiCol_Header] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.21f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.24f, 0.25f, 1.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.01f, 0.02f, 0.04f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.36f, 0.46f, 0.54f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.55f, 0.78f, 1.00f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.58f, 0.71f, 0.82f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.11f, 0.12f, 0.13f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.11f, 0.12f, 0.13f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.12f, 0.13f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.50f, 0.50f, 0.50f, 0.35f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);

        style.PopupRounding = 3;

        style.WindowPadding = ImVec2(6, 6);
        style.FramePadding = ImVec2(5, 4);
        style.ItemSpacing = ImVec2(8, 2);
        style.CellPadding = ImVec2(4, 1);
        style.ScrollbarSize = 15;
        style.WindowBorderSize = 1;
        style.ChildBorderSize = 0;
        style.PopupBorderSize = 1;
        style.FrameBorderSize = 1;
        style.WindowRounding = 3;
        style.ChildRounding = 0;
        style.FrameRounding = 3;
        style.ScrollbarRounding = 2;
        style.GrabRounding = 3;

        style.TabBorderSize = 0;
        style.TabRounding = 2;
        style.IndentSpacing = 10;

        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);

        colors[ImGuiCol_Tab] = colors[ImGuiCol_TitleBg];
        colors[ImGuiCol_TabHovered] = colors[ImGuiCol_WindowBg];
        colors[ImGuiCol_TabActive] = colors[ImGuiCol_WindowBg];
        colors[ImGuiCol_TabUnfocused] = colors[ImGuiCol_TitleBg];
        colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_WindowBg];
        colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        style.ScaleAllSizes(scaleFactor);

//       guizmo
		f32 guizmoScaleFactor = scaleFactor * 1.1;
		auto& guizmoSize  = ImGuizmo::GetStyle();
		new (&guizmoSize)  ImGuizmo::Style{};
//
		guizmoSize.CenterCircleSize = guizmoSize.CenterCircleSize * guizmoScaleFactor;
		guizmoSize.HatchedAxisLineThickness = guizmoSize.HatchedAxisLineThickness * guizmoScaleFactor;
		guizmoSize.RotationLineThickness = guizmoSize.RotationLineThickness * guizmoScaleFactor;
		guizmoSize.RotationOuterLineThickness = guizmoSize.RotationOuterLineThickness * guizmoScaleFactor;
		guizmoSize.ScaleLineCircleSize = guizmoSize.ScaleLineCircleSize * guizmoScaleFactor;
		guizmoSize.ScaleLineThickness = guizmoSize.ScaleLineThickness * guizmoScaleFactor;
		guizmoSize.TranslationLineArrowSize = guizmoSize.TranslationLineArrowSize * guizmoScaleFactor;
		guizmoSize.TranslationLineThickness = guizmoSize.TranslationLineThickness * guizmoScaleFactor;

//		ImNodesStyle& nodesStyle = ImNodes::GetStyle();
//		nodesStyle.GridSpacing = nodesStyle.GridSpacing * scaleFactor;
//		nodesStyle.NodeCornerRounding = nodesStyle.NodeCornerRounding * scaleFactor;
//		nodesStyle.NodePadding = nodesStyle.NodePadding * scaleFactor;
//		nodesStyle.NodeBorderThickness = nodesStyle.NodeBorderThickness * scaleFactor;
//		nodesStyle.LinkThickness = nodesStyle.LinkThickness * scaleFactor;
//		nodesStyle.LinkLineSegmentsPerLength = nodesStyle.LinkLineSegmentsPerLength * scaleFactor;
//		nodesStyle.LinkHoverDistance = nodesStyle.LinkHoverDistance * scaleFactor;
//		nodesStyle.PinCircleRadius = 4 * scaleFactor;
//		nodesStyle.PinQuadSideLength = nodesStyle.PinQuadSideLength * scaleFactor;
//		nodesStyle.PinTriangleSideLength = nodesStyle.PinTriangleSideLength * scaleFactor;
//		nodesStyle.PinLineThickness = nodesStyle.PinLineThickness * scaleFactor;
//		nodesStyle.PinHoverRadius = nodesStyle.PinHoverRadius * scaleFactor;
//		nodesStyle.PinOffset = nodesStyle.PinOffset * scaleFactor;
//		nodesStyle.MiniMapPadding = nodesStyle.MiniMapPadding * scaleFactor;
//		nodesStyle.MiniMapOffset = nodesStyle.MiniMapOffset * scaleFactor;
//
//		nodesStyle.Colors[ImNodesCol_TitleBar] = IM_COL32(53, 54, 56, 255);
//		nodesStyle.Colors[ImNodesCol_TitleBarSelected] = IM_COL32(83, 84, 86, 255);
//		nodesStyle.Colors[ImNodesCol_TitleBarHovered] = IM_COL32(73, 74, 76, 255);
//		nodesStyle.Colors[ImNodesCol_GridBackground] = IM_COL32(20, 21, 23, 255);
//		nodesStyle.Colors[ImNodesCol_GridLine] = IM_COL32(38, 39, 31, 255);
//		nodesStyle.Colors[ImNodesCol_NodeBackground] = IM_COL32(28, 31, 33, 255);
//		nodesStyle.Colors[ImNodesCol_NodeBackgroundSelected] = nodesStyle.Colors[ImNodesCol_NodeBackground];
//		nodesStyle.Colors[ImNodesCol_NodeBackgroundHovered] = nodesStyle.Colors[ImNodesCol_NodeBackground];
//		nodesStyle.Colors[ImNodesCol_NodeOutline] = nodesStyle.Colors[ImNodesCol_TitleBar];
    }

    void ApplyFonts()
    {
        f32 fontSize = 15.f;

        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();
        ResourceObject dejaVuSans = Repository::Read(Repository::GetByPath("Fyrion://Fonts/DejaVuSans.ttf"));
        if (dejaVuSans)
        {
            const Array<u8>& dejaVuSansBytes = dejaVuSans.GetValue<Array<u8>>(UIFont::FontBytes);

            auto font = ImFontConfig();
            font.SizePixels = fontSize * scaleFactor;
            memcpy(font.Name, "NotoSans", 10);
            font.FontDataOwnedByAtlas = false;
            io.Fonts->AddFontFromMemoryTTF((void*) dejaVuSansBytes.Data(), dejaVuSansBytes.Size(), font.SizePixels, &font);
        }
        else
        {
            ImFontConfig config{};
            config.SizePixels = fontSize * scaleFactor;
            io.Fonts->AddFontDefault(&config);
        }

        ResourceObject faSolid = Repository::Read(Repository::GetByPath("Fyrion://Fonts/fa-solid-900.otf"));
        if (faSolid)
        {
            static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
            static const ImWchar icon_ranges_[] = {ICON_MIN_FA + ICON_MAX_FA, ICON_MAX_FA + ICON_MAX_FA, 0};

            ImFontConfig config = ImFontConfig();
            config.SizePixels = fontSize * scaleFactor;
            config.MergeMode = true;
            config.GlyphMinAdvanceX = fontSize * scaleFactor;
            config.GlyphMaxAdvanceX = fontSize * scaleFactor;
            config.FontDataOwnedByAtlas = false;
            memcpy(config.Name, "FontAwesome", 11);

            const Array<u8>& faSolidBytes = faSolid.GetValue<Array<u8>>(UIFont::FontBytes);
            io.Fonts->AddFontFromMemoryTTF((void*) faSolidBytes.Data(), faSolidBytes.Size(), config.SizePixels, &config, icon_ranges);
        }
    }

    void Init(Fyrion::Window window, Fyrion::Swapchain swapchain)
    {
        scaleFactor = Platform::GetWindowScale(window);
        RegisterKeys();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        io.BackendPlatformName = "imgui_impl_fyrion";
        io.BackendRendererName = "imgui_impl_fyrion";
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigViewportsNoTaskBarIcon = true;

        ApplyDefaultStyle();
        ApplyFonts();

        Platform::ImGuiInit(window);
        GetRenderDevice().ImGuiInit(swapchain);
    }

    void BeginFrame(Fyrion::Window window, Fyrion::f64 deltaTime)
    {
        GetRenderDevice().ImGuiNewFrame();
        Platform::ImGuiNewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void ImGui::Render(RenderCommands& renderCommands)
    {
        ImGui::Render();
        GetRenderDevice().ImGuiRender(renderCommands);
    }

    void ImGuiShutdown()
    {
        contentTables.Clear();
        ImGui::DestroyContext();
    }

    ImGuiKey GetImGuiKey(Key key)
    {
        return keys[static_cast<usize>(key)];
    }

    void DrawType(ImGuiID itemId, TypeHandler* typeHandler, VoidPtr instance, bool* hasChanged)
    {
        //TODO
    }
}


