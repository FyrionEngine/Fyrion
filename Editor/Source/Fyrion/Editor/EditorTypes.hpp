#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{

    enum class DockPosition
    {
        None = 0,
        Center = 1,
        Left = 2,
        TopRight = 3,
        BottomRight = 4,
        Bottom = 5
    };

    struct EditorWindowProperties
    {
        DockPosition dockPosition{};
        bool createOnInit{};
    };


    struct EditorWindow
    {
        virtual EditorWindowProperties  GetProperties() { return {}; }
        virtual void                    Init(u32 id, VoidPtr userData) {}

        virtual void                    Draw(u32 id, bool& open) = 0;
    };

}