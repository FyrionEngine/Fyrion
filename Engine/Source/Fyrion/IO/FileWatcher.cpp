#include "FileWatcher.hpp"
#include "FileSystem.hpp"
#include <FileWatch.hpp>

#include <mutex>
#include <queue>

#include "Path.hpp"
#include "Fyrion/Core/Logger.hpp"

namespace Fyrion
{
    using FileWatchType = filewatch::FileWatch<std::string>;

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::FileWatcher", LogLevel::Debug);

        FileNotifyEvent CastEvent(filewatch::Event event)
        {
            switch (event)
            {
                case filewatch::Event::added:
                    return FileNotifyEvent::Added;
                case filewatch::Event::removed:
                    return FileNotifyEvent::Removed;
                case filewatch::Event::modified:
                    return FileNotifyEvent::Modified;
                case filewatch::Event::renamed_old:
                    return FileNotifyEvent::RenamedOld;
                case filewatch::Event::renamed_new:
                    return FileNotifyEvent::RenamedNew;
                    break;
            }
            return FileNotifyEvent::None;
        }
    }

    struct FileWatcherHandler
    {
        Array<std::unique_ptr<FileWatchType>> watches{};
        std::mutex                            mutex{};
        std::queue<FileWatcherModified>       modified{};
    };


    FileWatcher::FileWatcher()
    {
        handler = MemoryGlobals::GetDefaultAllocator().Alloc<FileWatcherHandler>();
    }

    void FileWatcher::Watch(const StringView& fileDir)
    {
        String fileDirPath = fileDir;
        handler->watches.EmplaceBack(std::make_unique<FileWatchType>(
            fileDir.begin(), [this, fileDirPath](const std::string& path, const filewatch::Event event)
            {
                std::unique_lock lockQueue(handler->mutex);
                handler->modified.emplace(FileWatcherModified{
                    .absolutePath = Path::Join(fileDirPath, String(path.c_str(), path.size())),
                    .event = CastEvent(event)
                });
            }
        ));

        logger.Debug("directory {} watched ", fileDir);
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
