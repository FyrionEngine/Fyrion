#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion
{
    struct FileStatus
    {
        bool    exists{};
        bool    isDirectory{};
        u64     lastModifiedTime{};
        u64     fileSize{};
    };

    class FY_API DirIterator
    {
    private:
        String  m_directory{};
        String  m_path{};
        VoidPtr m_handler{};
    public:
        DirIterator() = default;

        explicit DirIterator(const StringView& directory);

        String& operator*()
        {
            return m_path;
        }

        String* operator->()
        {
            return &m_path;
        }

        friend bool operator==(const DirIterator& a, const DirIterator& b)
        {
            return a.m_path == b.m_path;
        };

        friend bool operator!=(const DirIterator& a, const DirIterator& b)
        {
            return a.m_path != b.m_path;
        };

        DirIterator& operator++();

        virtual ~DirIterator();
    };

    class FY_API DirectoryEntries
    {

    private:
        String m_directory{};
    public:

        DirectoryEntries(const StringView& directory) : m_directory(directory)
        {}

        DirIterator begin()
        {
            return DirIterator{m_directory};
        }

        DirIterator end()
        {
            return DirIterator{};
        }
    };
}