#include "ImGui.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include "Fyrion/IO/InputTypes.hpp"
#include "ImGuiPlatform.hpp"
#include "Fyrion/Graphics/Device/RenderDevice.hpp"

using namespace Fyrion;

namespace Fyrion
{
    RenderDevice& GetRenderDevice();
}

namespace ImGui
{
    namespace
    {
        f32 scaleFactor = 1.0;
        ImGuiKey keys[static_cast<u32>(Key::MAX)];
    }

    void RegisterKeys()
    {
        keys[static_cast<u32>(Key::Space)] = ImGuiKey_Space;
        keys[static_cast<u32>(Key::Apostrophe)] =ImGuiKey_Apostrophe;
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
//		f32 guizmoScaleFactor = scaleFactor * 1.1;
//		auto& guizmoSize  = ImGuizmo::GetStyle();
//		new (&guizmoSize)  ImGuizmo::Style{};
//
//		guizmoSize.CenterCircleSize = guizmoSize.CenterCircleSize * guizmoScaleFactor;
//		guizmoSize.HatchedAxisLineThickness = guizmoSize.HatchedAxisLineThickness * guizmoScaleFactor;
//		guizmoSize.RotationLineThickness = guizmoSize.RotationLineThickness * guizmoScaleFactor;
//		guizmoSize.RotationOuterLineThickness = guizmoSize.RotationOuterLineThickness * guizmoScaleFactor;
//		guizmoSize.ScaleLineCircleSize = guizmoSize.ScaleLineCircleSize * guizmoScaleFactor;
//		guizmoSize.ScaleLineThickness = guizmoSize.ScaleLineThickness * guizmoScaleFactor;
//		guizmoSize.TranslationLineArrowSize = guizmoSize.TranslationLineArrowSize * guizmoScaleFactor;
//		guizmoSize.TranslationLineThickness = guizmoSize.TranslationLineThickness * guizmoScaleFactor;

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
        //ImGuizmo::BeginFrame();
    }

    void ImGui::Render(RenderCommands& renderCommands)
    {
        ImGui::Render();
        GetRenderDevice().ImGuiRender(renderCommands);
    }

    void ImGuiShutdown()
    {
        //ImGui::DestroyExtraContext();
        ImGui::DestroyContext();
    }

}


