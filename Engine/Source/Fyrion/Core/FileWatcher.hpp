#pragma once

#include "StringView.hpp"
#include "String.hpp"

#include <thread>
#include <mutex>
#include <optional>
#include <queue>
#include <condition_variable>

#include "Array.hpp"

namespace Fyrion
{
    enum class FileNotifyChange
    {
        Modified,
        Removed
    };

    typedef void (*FileWatcherCallbackFn)(VoidPtr fileRef, const String& path, const FileNotifyChange& notifyChange);

    struct FileWatcherData
    {
        String  path{};
        VoidPtr ref{};
        u64     lastModifiedTime{};
    };

    struct FileWatcherModified
    {
        String           path{};
        VoidPtr          ref{};
        FileNotifyChange change{};
    };

    class FileWatcher
    {
    public:
        FileWatcher();
        virtual ~FileWatcher();

        void Start();
        void Stop();

        void AddFile(VoidPtr fileRef, const StringView& fileDir);
        void CheckForUpdates(FileWatcherCallbackFn watcherCallbackFn);

    private:
        bool                            m_running{};
        std::optional<std::thread>      m_thread{};
        std::mutex                      m_mutex{};
        std::mutex                      m_mutex_queue{};
        Array<FileWatcherData>          m_files{};
        std::queue<FileWatcherModified> m_modified{};
        std::queue<FileWatcherData>     m_newFiles{};
        std::condition_variable         m_mutex_condition{};
        void                            ThreadLoop();
    };
}
