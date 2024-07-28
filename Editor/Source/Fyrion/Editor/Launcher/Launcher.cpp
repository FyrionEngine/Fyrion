#include "Launcher.hpp"

#include "Fyrion/Engine.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/ImGui/IconsFontAwesome6.h"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Fyrion/Platform/Platform.hpp"

#include "LauncherTypes.hpp"
#include "Fyrion/Editor/Editor.hpp"

#define PROJECT_NAME_EMPTY          2
#define PROJECT_ALREADY_EXISTS      3
#define PATH_EMPTY                  4

namespace Fyrion
{
    namespace
    {
        String                   projectPath{};
        bool                     creatingNewProject{};
        String                   projectSearch{};
        String                   appFolder{};
        String                   settingsFolder{};
        AssetDirectory*          settingsDir = nullptr;
        ProjectLauncherSettings* projectLauncherSettings = nullptr;

        String newProjectPath{};
        String newProjectName = "New Project";

        u32    projectNameValidation{0};
        u32    projectPathValidation{0};
        String selectedProject{};

        ImGuiID templateProjectId = 30000;

        Texture iconTexture = {};
    }


    void LauncherInit()
    {
        if (TextureAsset* textureAsset = AssetDatabase::FindByPath<TextureAsset>("Fyrion://Textures/LogoSmall.jpeg"))
        {
            iconTexture = textureAsset->GetTexture();
        }

        newProjectPath = Path::Join(FileSystem::DocumentsDir(), "Fyrion Projects");
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
                    String path{};
                    if (Platform::PickFolder(path, {}) == DialogResult::OK)
                    {
                        if (FileSystem::GetFileStatus(path).exists)
                        {
                            projectPath = path;
                            Engine::Shutdown();
                        }
                    }
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

                    auto c = 0;
                    for (const auto& recentProject : projectLauncherSettings->recentProjects)
                    {
                        //auto fileStat = fileSystem.fileStatus(recentProject);

                        String projectName = Path::Name(projectPath);

                        // Path file = recentProject;
                        // if (!searchText.Empty())
                        // {
                        //     auto fileName = file.Name();
                        //     ToUpper(fileName.begin(), fileName.end());
                        //     if (!Has(fileName.begin(), fileName.end(), searchText))
                        //     {
                        //         continue;
                        //     }
                        // }

                        c++;

                        ImGui::ContentItemDesc contentItem{};
                        contentItem.ItemId = 30001 + c;
                        contentItem.CanRename = false;
                        contentItem.ShowDetails = true;
                        contentItem.DetailsDesc = "Project";
                        contentItem.Label = projectName.CStr();
                        //contentItem.texture = projectTexture;

                        if (ImGui::DrawContentItem(contentItem))
                        {
                            projectPath = recentProject;
                            Engine::Shutdown();
                        }

                        // if (ImGui::ContentItemHovered(contentItem.itemId) && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
                        // {
                        //     openPopup = true;
                        // }
                        //
                        // if (ImGui::ContentItemFocused(contentItem.itemId))
                        // {
                        //     selectedProject = recentProject;
                        // }
                    }

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

                    ImGui::BeginContentTable(templateProjectId, 96 * ImGui::GetStyle().ScaleFactor, ImGuiContentTableFlags_SelectionNoFocus);

                    if (!ImGui::GetSelectedContentItem())
                    {
                        ImGui::SelectContentItem(30010, templateProjectId);
                    }

                    ImGui::ContentItemDesc contentItem{};
                    contentItem.ItemId = 30010;
                    contentItem.CanRename = false;
                    contentItem.ShowDetails = true;
                    contentItem.DetailsDesc = "Template";
                    contentItem.Label = "Default";
                    contentItem.Texture = iconTexture;

                    ImGui::DrawContentItem(contentItem);

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

                    ImGui::InputText(996633, newProjectName);

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
                        String path;
                        if (Platform::PickFolder(path, {}) == DialogResult::OK)
                        {
                            auto stat = FileSystem::GetFileStatus(path);
                            if (stat.exists)
                            {
                                newProjectPath = path;
                            }
                        }
                    }

                    ImGui::EndHorizontal();

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8 * ImGui::GetStyle().ScaleFactor);

                    ImGui::Spring(1);

                    ImGui::BeginDisabled(!valid);

                    if (ImGui::Button("OK", ImVec2(120, 0)) && valid)
                    {
                        projectPath = Editor::CreateProject(newProjectPath, newProjectName);
                        if (!projectPath.Empty())
                        {
                            projectLauncherSettings->defaultPath = newProjectPath;
                            projectLauncherSettings->recentProjects.EmplaceBack(projectPath);
                            projectLauncherSettings->SetModified();
                            Engine::Shutdown();
                        }
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

            if (ImGui::MenuItem(ICON_FA_TRASH " Remove")) {}
        }

        ImGui::EndPopupMenu(popupRes);

        ImGui::End();
    }

    String Launcher::GetProject()
    {
        return projectPath;
    }

    void OnLauncherShutdown()
    {
        AssetDatabase::SaveOnDirectory(settingsDir, settingsFolder);
    }

    void Launcher::Shutdown()
    {
        Event::Unbind<OnInit, &LauncherInit>();
        Event::Unbind<OnUpdate, &LauncherUpdate>();
        Event::Unbind<OnShutdown, &OnLauncherShutdown>();
    }

    void Launcher::Init()
    {
        Registry::Type<ProjectLauncherSettings>();

        appFolder = Path::Join(FileSystem::AppFolder(), "Fyrion");
        if (!FileSystem::GetFileStatus(appFolder).exists)
        {
            FileSystem::CreateDirectory(appFolder);
        }
        AssetDatabase::SetDataDirectory(Path::Join(appFolder, "Data"));

        settingsFolder = Path::Join(appFolder, "Settings");

        if (!FileSystem::GetFileStatus(settingsFolder).exists)
        {
            FileSystem::CreateDirectory(settingsFolder);
        }

        settingsDir = AssetDatabase::LoadFromDirectory("Settings", settingsFolder);

        projectLauncherSettings = AssetDatabase::FindByPath<ProjectLauncherSettings>("Settings://ProjectLauncherSettings.fy_asset");
        if (projectLauncherSettings == nullptr)
        {
            projectLauncherSettings = AssetDatabase::Create<ProjectLauncherSettings>();
            projectLauncherSettings->SetExtension(FY_ASSET_EXTENSION);
            projectLauncherSettings->SetUUID(UUID::RandomUUID());
            projectLauncherSettings->SetName("ProjectLauncherSettings");
            settingsDir->AddChild(projectLauncherSettings);
        }

        Event::Bind<OnInit, &LauncherInit>();
        Event::Bind<OnUpdate, &LauncherUpdate>();
        Event::Bind<OnShutdown, &OnLauncherShutdown>();


        EngineContextCreation contextCreation{
            .title = "Fyrion Launcher",
            .resolution = {1280, 720},
            .maximize = false,
            .headless = false,
        };
        Engine::CreateContext(contextCreation);
    }

    void ProjectLauncherSettings::RegisterType(NativeTypeHandler<ProjectLauncherSettings>& type)
    {
        type.Field<&ProjectLauncherSettings::defaultPath>("defaultPath");
        type.Field<&ProjectLauncherSettings::recentProjects>("recentProjects");
    }
}
