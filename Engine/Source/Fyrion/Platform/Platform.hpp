#pragma once

#include "PlatformTypes.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Math.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/IO/InputTypes.hpp"

namespace Fyrion::Platform
{
    FY_API void         ProcessEvents();
    FY_API void         WaitEvents();

    FY_API Window       CreateWindow(const StringView& title, const Extent& extent, WindowFlags flags);
    FY_API Extent       GetWindowExtent(Window window);
    FY_API bool         UserRequestedClose(Window window);
    FY_API void         DestroyWindow(Window window);
    FY_API f32          GetWindowScale(Window window);
    FY_API void         SetWindowShouldClose(Window window, bool shouldClose);
    FY_API void         SetClipboardString(Window window, StringView string);
    FY_API void         SetCursor(Window window, MouseCursor mouseCursor);

    FY_API f64          GetTime();
    FY_API f64          GetElapsedTime();

    FY_API void         ShowInExplorer(const StringView& path);

    FY_API DialogResult SaveDialog(String& path, const Span<FileFilter>& filters, const StringView& defaultPath, const StringView& fileName);
    FY_API DialogResult OpenDialog(String& path, const Span<FileFilter>& filters, const StringView& defaultPath);
    FY_API DialogResult OpenDialogMultiple(Array<String>& paths, const Span<FileFilter>& filters, const StringView& defaultPath);
    FY_API DialogResult PickFolder(String& path, const StringView& defaultPath);

    FY_API VoidPtr LoadDynamicLib(const StringView& library);
    FY_API void    FreeDynamicLib(VoidPtr library);
    FY_API VoidPtr GetFunctionAddress(VoidPtr library, const StringView& functionName);
}
