#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/Hash.hpp"

namespace Fyrion
{
    class SceneObject;

    constexpr const char* SceneTreePayload = "scene-tree-payload";

    using OnSceneObjectSelection = EventType<"Fyrion::Editor::OnSceneObjectSelection"_h, void(SceneObject*)>;

    enum class DockPosition
    {
        None = 0,
        Center = 1,
        Left = 2,
        TopRight = 3,
        BottomRight = 4,
        Bottom = 5
    };

    enum class PlayMode
    {
        Editing = 0,
        Paused = 1,
        Simulating = 2
    };

    struct EditorWindowProperties
    {
        DockPosition dockPosition{};
        bool createOnInit{};
    };


    struct EditorWindow
    {
        virtual void Init(u32 id, VoidPtr userData) {}
        virtual void Draw(u32 id, bool& open) = 0;
        virtual ~EditorWindow() = default;
    };

}