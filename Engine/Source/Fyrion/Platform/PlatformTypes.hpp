#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
    FY_HANDLER(Window);

    enum WindowFlags : i32
    {
        None          = 0,
        Maximized     = 1 << 0,
        Fullscreen    = 1 << 1
    };

    ENUM_FLAGS(WindowFlags, i32)

    enum class DialogResult
    {
        OK = 0,
        Error = 1,
        Cancel = 2
    };

    struct FileFilter
    {
        const char* name{};
        const char* spec{};
    };
}