#include "FileWatcher.hpp"
#include "FileSystem.hpp"

#include <mutex>
#include <queue>

#include "Fyrion/Core/Logger.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::FileWatcher", LogLevel::Debug);
    }

    struct FileWatcherHandler
    {
        std::mutex                            mutex{};
        std::queue<FileWatcherModified>       modified{};
    };


    FileWatcher::FileWatcher()
    {
        handler = MemoryGlobals::GetDefaultAllocator().Alloc<FileWatcherHandler>();
    }

    void FileWatcher::Watch(const StringView& fileDir)
    {

    }

    void FileWatcher::CheckForUpdates(FileWatcherCallbackFn watcherCallbackFn) const
    {
        std::unique_lock lockQueue(handler->mutex);
        while (!handler->modified.empty())
        {
            FileWatcherModified& data = handler->modified.front();
            watcherCallbackFn(data.absolutePath, data.event);
            handler->modified.pop();
        }
    }

    FileWatcher::~FileWatcher()
    {
        MemoryGlobals::GetDefaultAllocator().DestroyAndFree(handler);
    }
}
