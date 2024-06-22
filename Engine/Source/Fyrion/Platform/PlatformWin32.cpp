#include "Platform.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

#ifdef FY_WIN

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

namespace Fyrion
{

    void Platform::ShowInExplorer(const StringView& path)
    {
        auto stat = FileSystem::GetFileStatus(path);
        if (stat.isDirectory)
        {
            ShellExecute(NULL, "open", path.CStr(), NULL, NULL, SW_SHOWMAXIMIZED);
        }
        else
        {
            String parentPath = Path::Parent(path);
            ShellExecute(NULL, "open", parentPath.CStr(), NULL, NULL, SW_SHOWMAXIMIZED);
        }
    }

    VoidPtr Platform::LoadDynamicLib(const StringView& library)
    {
        if (Path::Extension(library).Empty())
        {
            String libName = String{library} + ".dll";
            return LoadLibrary(libName.CStr());
        }
        return LoadLibrary(library.CStr());
    }

    VoidPtr Platform::GetFunctionAddress(VoidPtr library, const StringView& functionName)
    {
        return (VoidPtr)GetProcAddress(static_cast<HMODULE>(library), functionName.CStr());
    }

    void Platform::FreeDynamicLib(VoidPtr library)
    {
        FreeLibrary(static_cast<HMODULE>(library));
    }
}

#endif