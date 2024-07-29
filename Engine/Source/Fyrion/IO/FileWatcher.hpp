#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/String.hpp"

namespace Fyrion
{
    enum class FileNotifyEvent
    {
        None,
        Added,
        Removed,
        Modified,
        RenamedOld,
        RenamedNew
    };


    struct FileWatcherData
    {
        String  path{};
        VoidPtr ref{};
        u64     lastModifiedTime{};
    };

    struct FileWatcherModified
    {
        String          absolutePath{};
        FileNotifyEvent event{};
    };

    typedef void (*FileWatcherCallbackFn)(const StringView& absolutePath, FileNotifyEvent event);

    struct FileWatcherHandler;

    class FileWatcher
    {
    public:
        FY_NO_COPY_CONSTRUCTOR(FileWatcher);

        FileWatcher();
        virtual ~FileWatcher();

        void Watch(const StringView& fileDir);
        void CheckForUpdates(FileWatcherCallbackFn watcherCallbackFn) const;

    private:
        FileWatcherHandler* handler = nullptr;
    };
}
