#include "ImGuiEditor.hpp"

#include <functional>

#include "imgui_internal.h"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Attributes.hpp"
#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/ImGui/ImGui.hpp"


namespace
{
    struct AssetSelector
    {
        TypeID                         assetId;
        VoidPtr                        userData;
        ImGui::FnAssetSelectorCallback callback;
    };

    Array<AssetSelector>                              assetSelectors;
    HashMap<usize, SharedPtr<ImGui::DrawTypeContent>> drawTypes{};
    Array<ImGui::FieldRendererFn>                     fieldRenders{};
    usize                                             renamingItem = 0;

    void DrawAssetSelection(bool& showAssetSelection, TypeID assetType, VoidPtr userData, ImGui::FnAssetSelectorCallback callback)
    {
        auto& style = ImGui::GetStyle();
        auto& io = ImGui::GetIO();

        static String searchResoruceString = "";
        static bool   popupOpen = false;

        auto originalPadding = ImGui::GetStyle().WindowPadding;

        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        if (!popupOpen)
        {
            popupOpen = true;
            ImGui::OpenPopup("Assets");

            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(960 * ImGui::GetStyle().ScaleFactor, 540 * ImGui::GetStyle().ScaleFactor), ImGuiCond_Appearing);
        }

        if (ImGui::BeginPopupModal("Assets", &showAssetSelection))
        {
            {
                ImGui::StyleVar windowPadding2(ImGuiStyleVar_WindowPadding, originalPadding);
                ImGui::BeginChild(1000, ImVec2(0, (25 * style.ScaleFactor) + originalPadding.y), false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);

                ImGui::SetNextItemWidth(-1);
                ImGui::SearchInputText(12471247, searchResoruceString);
                ImGui::EndChild();
            }

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + originalPadding.y);
            auto  p1 = ImGui::GetCursorScreenPos();
            auto  p2 = ImVec2(ImGui::GetContentRegionAvail().x + p1.x, p1.y);
            auto* drawList = ImGui::GetWindowDrawList();
            drawList->AddLine(p1, p2, ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Separator]), 1.f * style.ScaleFactor);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.f * style.ScaleFactor);

            {
                ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(22, 23, 25, 255));
                ImGui::StyleVar   windowPadding2(ImGuiStyleVar_WindowPadding, originalPadding);

                static f32 zoom = 1.0;

                ImGui::SetWindowFontScale(zoom);

                if (ImGui::BeginChild(10000, ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
                {
                    if (ImGui::BeginContentTable("asset-selection", zoom))
                    {
                        {
                            ImGui::ContentItemDesc contentItem{};
                            contentItem.id = HashValue("None-Id");
                            contentItem.label = "None";
                            contentItem.thumbnailScale = zoom;

                            ImGui::ContentItemState state = ImGui::ContentItem(contentItem);

                            if (state.doubleClicked)
                            {
                                showAssetSelection = false;
                                callback(userData, nullptr);
                            }
                        }

                        std::function<void(TypeID typeId)> drawAssetSelection;
                        drawAssetSelection = [&](const TypeID typeId)
                        {
                            for (AssetFile* assetFile : AssetEditor::GetAssetsOfType(typeId))
                            {
                                ImGui::ContentItemDesc contentItem{};
                                contentItem.id = assetFile->hash;
                                contentItem.label = assetFile->fileName;
                                contentItem.thumbnailScale = zoom;
                                contentItem.texture = assetFile->GetThumbnail();

                                ImGui::ContentItemState state = ImGui::ContentItem(contentItem);

                                if (state.doubleClicked)
                                {
                                    showAssetSelection = false;
                                    callback(userData, assetFile);
                                }
                            }

                            //??
                            // if (TypeHandler* typeHandler = Registry::FindTypeById(typeId))
                            // {
                            //     for (DerivedType derived : typeHandler->GetDerivedTypes())
                            //     {
                            //         drawAssetSelection(derived.typeId);
                            //     }
                            // }
                        };
                        drawAssetSelection(assetType);
                        ImGui::EndContentTable();
                    }
                    ImGui::EndChild();
                    ImGui::SetWindowFontScale(1);
                }
            }

            ImGui::EndPopup();
        }

        if (ImGui::IsKeyDown(ImGuiKey_Escape))
        {
            showAssetSelection = false;
        }

        if (!showAssetSelection)
        {
            searchResoruceString.Clear();
            popupOpen = false;
        }
    }
}


namespace Fyrion
{
    void ImGuiUpdate()
    {
        if (!assetSelectors.Empty())
        {
            AssetSelector& selector = assetSelectors.Back();

            bool show = true;
            DrawAssetSelection(show, selector.assetId, selector.userData, selector.callback);
            if (!show)
            {
                assetSelectors.PopBack();
            }
        }

        u64 frame = Engine::GetFrame();

        Array<u64> toErase{};
        for (auto& it : drawTypes)
        {
            if (it.second->hasChanged)
            {
                if (it.second->desc.callback)
                {
                    it.second->desc.callback(it.second->desc, it.second->instance);
                }
                it.second->hasChanged = false;
            }

            if (it.second->lastFrameUsage + 60 < frame)
            {
                if (!it.second->readOnly)
                {
                    it.second->desc.typeHandler->Destroy(it.second->instance);
                }
                toErase.EmplaceBack(it.first);
            }
        }

        for (u64 id : toErase)
        {
            drawTypes.Erase(id);
        }
    }

    void ImGuiShutdown()
    {
        drawTypes.Clear();
        fieldRenders.Clear();
    }
}

void ImGui::ShowAssetSelector(TypeID assetId, VoidPtr userData, FnAssetSelectorCallback callback)
{
    assetSelectors.EmplaceBack(assetId, userData, callback);
}

void ImGui::AddFieldRenderer(FieldRendererFn fieldRendererFn)
{
    fieldRenders.EmplaceBack(fieldRendererFn);
}

Span<ImGui::FieldRendererFn> ImGui::GetFieldRenderers()
{
    return fieldRenders;
}

void ImGui::DrawType(const DrawTypeDesc& desc)
{
    bool readOnly = desc.flags & ImGuiDrawTypeFlags_ReadOnly;

    auto it = drawTypes.Find(desc.itemId);

    if (it != drawTypes.end() && it->second->desc.typeHandler->GetTypeInfo().typeId != desc.typeHandler->GetTypeInfo().typeId)
    {
        if (!it->second->readOnly && it->second->instance != nullptr)
        {
            it->second->desc.typeHandler->Destroy(it->second->instance);
        }
        drawTypes.Erase(it);
    }

    if (it == drawTypes.end())
    {
        it = drawTypes.Emplace(
            desc.itemId,
            MakeShared<DrawTypeContent>(
                DrawTypeContent{
                    .desc = desc,
                    .instance = !readOnly ? desc.typeHandler->NewInstance() : const_cast<VoidPtr>(desc.instance),
                    .readOnly = readOnly,
                    .tableRender = true
                })).first;

        desc.typeHandler->DeepCopy(desc.instance, it->second->instance);
    }

    DrawTypeContent* content = it->second.Get();
    content->idCount = 15000;

    content->lastFrameUsage = Engine::GetFrame();

    if (!content->desc.typeHandler->GetFields().Empty())
    {
        if (BeginTable("##component-table", 2))
        {
            TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.6f);
            TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch);

            for (FieldHandler* field : content->desc.typeHandler->GetFields())
            {
                if (!field->HasAttribute<UIProperty>()) continue;
                content->activeFieldHandler = field;

                BeginDisabled(readOnly);

                TableNextColumn();
                AlignTextToFramePadding();

                String formattedName = FormatName(field->GetName());
                Text("%s", formattedName.CStr());
                TableNextColumn();


                VoidPtr fieldPointer = field->GetFieldPointer(content->instance);
                for (FieldRendererFn render : fieldRenders)
                {
                    render(content, field->GetFieldInfo().typeInfo, fieldPointer, &content->hasChanged);
                }

                EndDisabled();
            }
            EndTable();
        }
    }
}

void ImGui::ClearDrawData(VoidPtr ptr, bool clearActiveId)
{
    if (auto it = drawTypes.Find(reinterpret_cast<usize>(ptr)))
    {
        it->second->desc.typeHandler->DeepCopy(it->second->desc.instance, it->second->instance);
    }
    if (clearActiveId)
    {
        ClearActiveID();
    }
}

bool ImGui::BeginContentTable(const char* tableId, f32 thumbnailScale)
{
    u32 thumbnailSize = (thumbnailScale * 112.f) * ImGui::GetStyle().ScaleFactor;

    static ImGuiTableFlags tableFlags = ImGuiTableFlags_SizingFixedSame;
    u32                    columns = Math::Max(static_cast<u32>((ImGui::GetContentRegionAvail().x - GetStyle().WindowPadding.x) / thumbnailSize), 1u);

    bool ret = ImGui::BeginTable(tableId, columns, tableFlags);
    if (ret)
    {
        for (int i = 0; i < columns; ++i)
        {
            char buffer[20]{};
            sprintf(buffer, "%ld", i);
            TableSetupColumn(buffer, ImGuiTableColumnFlags_WidthFixed, thumbnailSize);
        }
    }
    return ret;
}

ImGui::ContentItemState ImGui::ContentItem(const ContentItemDesc& contentItemDesc)
{
    ImGuiStyle& style = ImGui::GetStyle();
    u32         thumbnailSize = (contentItemDesc.thumbnailScale * 112.f) * ImGui::GetStyle().ScaleFactor;

    ImGui::TableNextColumn();
    auto* drawList = ImGui::GetWindowDrawList();
    auto  screenCursorPos = ImGui::GetCursorScreenPos();

    f32 imagePadding = thumbnailSize * 0.08f;

    auto posEnd = ImVec2(screenCursorPos.x + thumbnailSize, screenCursorPos.y + thumbnailSize);
    bool hovered = ImGui::IsMouseHoveringRect(screenCursorPos, posEnd, true) && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup);

    i32  mouseCount = ImGui::GetMouseClickedCount(ImGuiMouseButton_Left);
    bool isDoubleClicked = mouseCount >= 2 && (mouseCount % 2) == 0 && hovered;
    bool isClicked = (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && hovered;

    if (hovered)
    {
        drawList->AddRectFilled(screenCursorPos,
                                posEnd,
                                IM_COL32(40, 41, 43, 255),
                                0.0f);
    }

    ContentItemState state = ContentItemState{
        .hovered = hovered,
        .clicked = isClicked,
        .doubleClicked = isDoubleClicked,
        .screenStartPos = screenCursorPos,
        .size = ImVec2(thumbnailSize, thumbnailSize)
    };

    ImGui::DrawTexture(contentItemDesc.texture, {
                           i32(screenCursorPos.x + imagePadding * 2),
                           i32(screenCursorPos.y + imagePadding),
                           (u32)(screenCursorPos.x + thumbnailSize - imagePadding * 2),
                           (u32)(screenCursorPos.y + thumbnailSize - imagePadding * 3)
                       });

    //rect size for texture
    ImGui::Dummy(ImVec2{(f32)thumbnailSize, (f32)thumbnailSize - imagePadding * 3.f});

    ImGui::BeginVertical(contentItemDesc.id + 10, ImVec2(thumbnailSize, thumbnailSize - (ImGui::GetCursorScreenPos().y - screenCursorPos.y)));
    {
        ImGui::Spring();

        auto textSize = ImGui::CalcTextSize(contentItemDesc.label.CStr());
        auto textPadding = textSize.y / 1.5f;
        textSize.x = Math::Min(textSize.x, f32(thumbnailSize - textPadding));

        ImGui::BeginHorizontal(contentItemDesc.id + 20, ImVec2(thumbnailSize, 0.0f));
        ImGui::Spring(1.0);
        {
            if (!contentItemDesc.renameItem)
            {
                ImGui::PushClipRect(GetCursorScreenPos(), GetCursorScreenPos() + textSize, true);
                drawList->AddText(GetCursorScreenPos(), GetColorU32(ImGuiCol_Text), contentItemDesc.label.CStr());
                ImGui::PopClipRect();
                ImGui::Dummy(textSize);
            }
            else
            {
                static String renameStringCache = {};
                ImGui::SetNextItemWidth(thumbnailSize - textPadding);

                if (renamingItem == 0)
                {
                    renameStringCache = contentItemDesc.label;
                    ImGui::SetKeyboardFocusHere();
                }

                ImGui::StyleColor frameColor(ImGuiCol_FrameBg, IM_COL32(52, 53, 55, 255));
                ImGui::InputText(contentItemDesc.id, renameStringCache);

                if (!ImGui::IsItemActive() && renamingItem != 0)
                {
                    if (!ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                    {
                        state.newName = renameStringCache;
                    }
                    state.renameFinish = true;
                    renamingItem = 0;
                }
                else if (renamingItem == 0)
                {
                    renamingItem = contentItemDesc.id;
                }
            }
        }

        ImGui::Spring(1.0);
        ImGui::EndHorizontal();
        ImGui::Spring();
    }
    ImGui::EndVertical();


    if (contentItemDesc.selected)
    {
        //selected
        drawList->AddRect(ImVec2(screenCursorPos.x, screenCursorPos.y),
                          ImVec2(screenCursorPos.x + thumbnailSize - 1, ImGui::GetCursorScreenPos().y - 1),
                          ImGui::ColorConvertFloat4ToU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f)),
                          0.0f, 0, 2);
    }

    return state;
}

void ImGui::EndContentTable()
{
    ImGui::EndTable();
}

void ImGui::TextWithLabel(StringView label, StringView text)
{
    ImGui::TextDisabled(label.CStr());
    ImGui::SameLine();
    ImGui::Text(text.CStr());
}
