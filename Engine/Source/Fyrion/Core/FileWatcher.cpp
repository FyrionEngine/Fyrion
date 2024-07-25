#include <iostream>
#include "FileWatcher.hpp"
#include <chrono>

#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/FileTypes.hpp"

namespace Fyrion
{
    FileWatcher::FileWatcher() {}

    void FileWatcher::ThreadLoop()
    {
        while (m_running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            {
                {
                    std::unique_lock lock(m_mutex);
                    while (!m_newFiles.empty())
                    {
                        m_files.EmplaceBack(m_newFiles.front());
                        m_newFiles.pop();
                    }
                }

                auto it = m_files.begin();
                while (it != m_files.end())
                {
                    FileWatcherData& data = *it;
                    FileStatus       fileStatus = FileSystem::GetFileStatus(data.path);

                    if (!fileStatus.exists)
                    {
                        std::unique_lock lockQueue(m_mutex_queue);
                        m_modified.emplace(FileWatcherModified{
                            .path = data.path,
                            .ref = data.ref,
                            .change = FileNotifyChange::Removed
                        });
                        it = m_files.Erase(it);
                    }
                    else if (fileStatus.lastModifiedTime != data.lastModifiedTime)
                    {
                        data.lastModifiedTime = fileStatus.lastModifiedTime;
                        {
                            std::unique_lock lockQueue(m_mutex_queue);
                            m_modified.emplace(FileWatcherModified{
                                .path = data.path,
                                .ref = data.ref,
                                .change = FileNotifyChange::Modified
                            });
                        }
                        ++it;
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
        }
    }

    void FileWatcher::Start()
    {
        Stop();
        m_running = true;
        m_thread = std::make_optional(std::thread(&FileWatcher::ThreadLoop, this));
    }

    void FileWatcher::AddFile(VoidPtr fileRef, const StringView& fileDir)
    {
        std::unique_lock lock(m_mutex);
        m_newFiles.emplace(FileWatcherData{
            .path = fileDir,
            .ref = fileRef,
            .lastModifiedTime = FileSystem::GetFileStatus(fileDir).lastModifiedTime
        });
    }

    void FileWatcher::CheckForUpdates(FileWatcherCallbackFn watcherCallbackFn)
    {
        std::unique_lock lockQueue(m_mutex_queue);
        while (!m_modified.empty())
        {
            FileWatcherModified& data = m_modified.front();
            watcherCallbackFn(data.ref, data.path, data.change);
            m_modified.pop();
        }
    }

    void FileWatcher::Stop()
    {
        m_running = false;
        if (m_thread)
        {
            m_thread.value().join();
            m_thread.reset();
        }
        m_files.Clear();
    }

    FileWatcher::~FileWatcher()
    {
        Stop();
    }
}
