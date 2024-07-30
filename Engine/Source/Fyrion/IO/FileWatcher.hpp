#pragma once
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/String.hpp"

namespace Fyrion
{
    enum class FileNotifyEvent
    {
        Added,
        Removed,
        Modified,
        Renamed,
    };


    struct FileWatcherModified
    {
        VoidPtr         userData{};
        String          oldName{};
        String          name{};
        String          path{};
        FileNotifyEvent event{};
    };

    typedef void (*FileWatcherCallbackFn)(const FileWatcherModified& modified);

    struct FileWatcherInternal;

    class FileWatcher
    {
    public:
        FY_NO_COPY_CONSTRUCTOR(FileWatcher);

        FileWatcher() = default;

        void Start();
        void Stop();
        void Check();

        void Watch(VoidPtr userData, const StringView& fileDir);
        void CheckForUpdates(FileWatcherCallbackFn watcherCallbackFn) const;

    private:
        FileWatcherInternal* internal = nullptr;
    };
}
