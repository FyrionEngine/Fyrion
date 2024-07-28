#include "Launcher.hpp"

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Fyrion/Platform/Platform.hpp"

#define PROJECT_NAME_EMPTY          2
#define PROJECT_ALREADY_EXISTS      3
#define PATH_EMPTY                  4

namespace Fyrion
{
    namespace
    {
        String projectPath{};
        bool   creatingNewProject{};
        String projectSearch{};

        String newProjectPath{};
        String newProjectName{};

        u32    projectNameValidation{0};
        u32    projectPathValidation{0};
        String selectedProject{};

        Texture iconTexture = {};
    }


    void LauncherInit()
    {
        if (TextureAsset* textureAsset = AssetDatabase::FindByPath<TextureAsset>("Fyrion://Textures/LogoSmall.jpeg"))
        {
            iconTexture = textureAsset->GetTexture();
        }
    }


    void LauncherUpdate(f64 deltaTime)
    {
        auto& style = ImGui::GetStyle();
        auto  padding = style.WindowPadding;

        ImGui::StyleVar itemSpacing(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::StyleVar windowPadding(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::BeginFullscreen(5000);

        auto listOptionsPanelSize = ImGui::GetContentRegionAvail().x * 0.2f;
        auto availableSpace = ImGui::GetContentRegionAvail().x - listOptionsPanelSize;

        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.1f, 0.5f));
        if (ImGui::BeginChild(52010, ImVec2(listOptionsPanelSize, 0)))
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding.y);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding.y);

            ImGui::TextureItem(iconTexture, ImVec2(48 * ImGui::GetStyle().ScaleFactor, 48 * ImGui::GetStyle().ScaleFactor));

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + padding.y);

            ImGui::Separator();

            auto buttonSize = ImVec2(listOptionsPanelSize, 35 * style.ScaleFactor);

            if (ImGui::Selectable(ICON_FA_DIAGRAM_PROJECT " Projects", true, ImGuiSelectableFlags_SpanAllColumns, buttonSize)) {}

            if (ImGui::Selectable(ICON_FA_PUZZLE_PIECE " Plugins", false, ImGuiSelectableFlags_SpanAllColumns, buttonSize)) {}
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(); //ImGuiStyleVar_SelectableTextAlign
        ImGui::SameLine();

        bool openPopup = false;

        if (!creatingNewProject)
        {
            ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(22, 23, 25, 255));
            ImGui::StyleColor frameBg(ImGuiCol_FrameBg, IM_COL32(22, 23, 25, 255));
            ImGui::StyleVar   frameBorderSize(ImGuiStyleVar_FrameBorderSize, 0.f);

            if (ImGui::BeginChild(52020, ImVec2(0, 0)))
            {
                auto buttonSize = ImVec2(100 * ImGui::GetStyle().ScaleFactor, 25 * ImGui::GetStyle().ScaleFactor);
                auto width = ImGui::GetContentRegionAvail().x - (buttonSize.x * 2.f) - (25 * ImGui::GetStyle().ScaleFactor);

                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + padding.x, ImGui::GetCursorPos().y + padding.y));

                ImGui::SetNextItemWidth(width);

                ImGui::SearchInputText(80005, projectSearch);

                ImGui::SameLine();

                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding.x);
                if (ImGui::Button("Open", buttonSize))
                {
                    // Path path{};
                    // auto res = Platform::PickFolder(path, {});
                    //
                    // if (res == DialogResult_Okay && FileSystem::FileStatus(path).exists)
                    // {
                    // 	projectLauncherSettings->recentProjects.EmplaceBack(path.GetString());
                    // 	SettingsServer::SaveSettings<ProjectLauncherSettings>();
                    //
                    // 	projectPath = path.GetString();
                    // 	App::Shutdown();
                    // }
                }

                ImGui::SameLine();
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + padding.x);

                if (ImGui::Button("New Project", buttonSize))
                {
                    creatingNewProject = true;
                    projectNameValidation = 0;
                    projectPathValidation = 0;
                }

                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + padding.x, ImGui::GetCursorPos().y + padding.y));

                ImGui::Separator();

                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + padding.x * 1.5f, ImGui::GetCursorPos().y + padding.y * 1.5f));
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.01f, 0.5f));

                    ImGui::BeginContentTable(30001, 96 * ImGui::GetStyle().ScaleFactor);

                    String searchText{};
                    if (!projectSearch.Empty())
                    {
                        searchText = projectSearch;
                        //ToUpper(searchText.begin(), searchText.end());
                    }

                    // auto c = 0;
                    // for (const auto& recentProject : projectLauncherSettings->recentProjects)
                    // {
                    //     //auto fileStat = fileSystem.fileStatus(recentProject);
                    //
                    //     Path file = recentProject;
                    //     if (!searchText.Empty())
                    //     {
                    //         auto fileName = file.Name();
                    //         ToUpper(fileName.begin(), fileName.end());
                    //         if (!Has(fileName.begin(), fileName.end(), searchText))
                    //         {
                    //             continue;
                    //         }
                    //     }
                    //
                    //     c++;
                    //
                    //     ImGui::UIContentItem contentItem{};
                    //     contentItem.itemId = 30001 + c;
                    //     contentItem.canRename = false;
                    //     contentItem.showDetails = true;
                    //     contentItem.detailsDesc = "Project";
                    //     contentItem.label = file.Name().CStr();
                    //     contentItem.texture = projectTexture;
                    //
                    //     if (ImGui::ContentItem(contentItem))
                    //     {
                    //         projectPath = recentProject;
                    //         App::Shutdown();
                    //     }
                    //
                    //     if (ImGui::ContentItemHovered(contentItem.itemId) && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
                    //     {
                    //         openPopup = true;
                    //     }
                    //
                    //     if (ImGui::ContentItemFocused(contentItem.itemId))
                    //     {
                    //         selectedProject = recentProject;
                    //     }
                    // }
                    //
                    ImGui::EndContentTable();
                    ImGui::PopStyleVar(); //ImGuiStyleVar_SelectableTextAlign
                }
            }
            ImGui::EndChild();
        }
        else if (creatingNewProject)
        {
            {
                ImGui::StyleColor childBg(ImGuiCol_ChildBg, IM_COL32(22, 23, 25, 255));
                ImGui::StyleColor frameBg(ImGuiCol_FrameBg, IM_COL32(22, 23, 25, 255));
                ImGui::StyleVar   frameBorderSize(ImGuiStyleVar_FrameBorderSize, 0.f);
                ImGui::StyleVar   windowPadding(ImGuiStyleVar_WindowPadding, padding);

                if (ImGui::BeginChild(52020, ImVec2(availableSpace * 0.6, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
                {
                    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + padding.x * 1.5f, ImGui::GetCursorPos().y + padding.y * 1.5f));

                    ImGuiID templateProjectId = 30201;

                    ImGui::BeginContentTable(templateProjectId, 96 * ImGui::GetStyle().ScaleFactor, ImGuiContentTableFlags_SelectionNoFocus);

                    // if (ImGui::GetContentItemSelected(templateProjectId) == U32_MAX)
                    // {
                    //     ImGui::SelectContentItem(30204, templateProjectId);
                    // }

                    {
                        ImGui::ContentItemDesc contentItem{};
                        contentItem.ItemId = 30204;
                        contentItem.CanRename = false;
                        contentItem.ShowDetails = true;
                        contentItem.DetailsDesc = "Template";
                        contentItem.Label = "Empty";
                        //   contentItem.Texture = iconTexture;

                        if (ImGui::DrawContentItem(contentItem)) {}
                    }

                    ImGui::EndContentTable();
                }
                ImGui::EndChild();
            }

            ImGui::SameLine();
            {
                ImGui::StyleVar childPadding(ImGuiStyleVar_WindowPadding, padding);
                if (ImGui::BeginChild(520245, ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
                {
                    projectNameValidation = 0;
                    projectPathValidation = 0;

                    String projectFullPath = Path::Join(newProjectPath, newProjectName);

                    bool valid = true;
                    if (newProjectName.Empty())
                    {
                        projectNameValidation = PROJECT_NAME_EMPTY;
                        valid = false;
                    }
                    else
                    {
                        auto stat = FileSystem::GetFileStatus(projectFullPath);
                        if (stat.exists)
                        {
                            projectNameValidation = PROJECT_ALREADY_EXISTS;
                            valid = false;
                        }
                    }

                    if (newProjectPath.Empty())
                    {
                        projectPathValidation = PATH_EMPTY;
                        valid = false;
                    }

                    auto h = ImGui::GetContentRegionAvail().y;

                    ImGui::BeginVertical(5555, ImVec2(0, h));
                    ImGui::Text("Project Name:");

                    if (projectNameValidation == PROJECT_NAME_EMPTY)
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.f), " Project Name is mandatory");
                    }
                    else if (projectNameValidation == PROJECT_ALREADY_EXISTS)
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.f), " Project already exists");
                    }


                    ImGui::SetNextItemWidth(-1);

                    ImGui::InputText(99663328, newProjectName);

                    ImGui::Text("Project Path:");

                    if (projectPathValidation == PATH_EMPTY)
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.f), " Project Path is mandatory");
                    }


                    ImGui::BeginHorizontal(55551);

                    ImGui::SetNextItemWidth(-60 * ImGui::GetStyle().ScaleFactor);

                    ImGui::InputText(99663328, newProjectPath);

                    ImGui::Spring(1);

                    if (ImGui::Button("Browse"))
                    {
                        // Path path{};
                        // if (Platform::PickFolder(path, {}) == DialogResult_Okay)
                        // {
                        //     auto stat = FileSystem::FileStatus(path);
                        //     if (stat.exists)
                        //     {
                        //         newProjectPath = path.GetString();
                        //     }
                        // }
                    }

                    ImGui::EndHorizontal();

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8 * ImGui::GetStyle().ScaleFactor);

                    ImGui::Spring(1);

                    ImGui::BeginDisabled(!valid);

                    if (ImGui::Button("OK", ImVec2(120, 0)) && valid)
                    {
                        // projectLauncherSettings->defaultPath = newProjectPath;
                        // projectLauncherSettings->recentProjects.EmplaceBack(projectFullPath);
                        // SettingsServer::SaveSettings<ProjectLauncherSettings>();
                        //
                        // ProjectServer::CreateProject(projectFullPath);
                        //
                        // projectPath = projectFullPath;
                        // App::Shutdown();
                    }

                    ImGui::EndDisabled();

                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();


                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    {
                        creatingNewProject = false;
                    }

                    ImGui::EndVertical();
                }
                ImGui::EndChild();
            }
        }

        if (!selectedProject.Empty() && openPopup)
        {
            ImGui::OpenPopup("project-browser-popup");
        }

        auto popupRes = ImGui::BeginPopupMenu("project-browser-popup");

        if (popupRes)
        {
            if (ImGui::MenuItem(ICON_FA_FOLDER " Show in Explorer"))
            {
                Platform::ShowInExplorer(selectedProject);
            }

            if (ImGui::MenuItem(ICON_FA_TRASH " Remove"))
            {
                // projectLauncherSettings->recentProjects.Erase(
                //     std::remove(projectLauncherSettings->recentProjects.begin(), projectLauncherSettings->recentProjects.end(), selectedProject),
                //     projectLauncherSettings->recentProjects.end());
                //
                // SettingsServer::SaveSettings<ProjectLauncherSettings>();
            }
        }

        ImGui::EndPopupMenu(popupRes);

        ImGui::End();
    }

    String Launcher::GetSelectedProject()
    {
        return {};
    }


    void Launcher::Init()
    {
        String appFolder = Path::Join(FileSystem::AppFolder(), "Fyrion");
        if (!FileSystem::GetFileStatus(appFolder).exists)
        {
            FileSystem::CreateDirectory(appFolder);
        }
        AssetDatabase::SetDataDirectory(Path::Join(appFolder, "Data"));

        Event::Bind<OnInit, &LauncherInit>();
        Event::Bind<OnUpdate, &LauncherUpdate>();

        EngineContextCreation contextCreation{
            .title = "Fyrion Launcher",
            .resolution = {1024, 600},
            .maximize = false,
            .headless = false,
        };
        Engine::CreateContext(contextCreation);
    }
}
