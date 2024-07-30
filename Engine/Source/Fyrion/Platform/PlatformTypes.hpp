#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Hash.hpp"
#include "Fyrion/Core/Event.hpp"

namespace Fyrion
{
    FY_HANDLER(Window);

    using OnDropFileCallback = EventType<"Fyrion::OnDropFileCallback"_h, void(Window window, const StringView& path)>;

    enum class WindowFlags : i32
    {
        None           = 0,
        Maximized      = 1 << 0,
        Fullscreen     = 1 << 1,
        SubscriveInput = 1 << 2,
    };

    ENUM_FLAGS(WindowFlags, i32)

    enum class DialogResult
    {
        OK     = 0,
        Error  = 1,
        Cancel = 2
    };

    struct FileFilter
    {
        const char* name{};
        const char* spec{};
    };
}
