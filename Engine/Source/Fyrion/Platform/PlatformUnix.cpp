
#include "Platform.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Fyrion/Core/Logger.hpp"

#ifdef FY_UNIX

#include <dlfcn.h>

namespace Fyrion
{

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::Platform");
    }

    namespace Platform
    {
        void InitStyle();
        void ApplyDarkStyle(VoidPtr internal);
    }

    void Platform::InitStyle()
    {

    }

    void Platform::ApplyDarkStyle(VoidPtr internal)
    {

    }

    f64 Platform::GetTime()
    {
        struct timespec now{};
        clock_gettime(CLOCK_MONOTONIC_RAW, &now);
        return now.tv_sec + now.tv_nsec * 0.000000001;
    }

    void Platform::ShowInExplorer(const StringView& path)
    {
#ifdef FY_LINUX
        if (FileSystem::GetFileStatus(path).isDirectory)
        {
            String command = "xdg-open " + String{path};
            system(command.CStr());
        }
        else
        {
            String command = "xdg-open " + Path::Parent(path);
            system(command.CStr());
        }
#else
#endif
    }

    VoidPtr Platform::LoadDynamicLib(const StringView& library)
    {
        VoidPtr ptr;
        if (Path::Extension(library).Empty())
        {
            String libName = "lib" + String{library} + FY_SHARED_EXT;
            ptr = dlopen(libName.CStr(), RTLD_NOW);
        }
        else
        {
            ptr = dlopen(library.CStr(), RTLD_NOW);
        }

        if (ptr == nullptr)
        {
            const char* err = dlerror();
            logger.Error("error on load dynamic lib {} ", err);
        }

        return ptr;
    }

    VoidPtr Platform::GetFunctionAddress(VoidPtr library, const StringView& functionName)
    {
        return dlsym(library, functionName.CStr());
    }

    void Platform::FreeDynamicLib(VoidPtr library)
    {
        dlclose(library);
    }
}

#endif