#include "FileSystem.hpp"
#include "Path.hpp"

#ifdef FY_WIN

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <direct.h>
#include <KnownFolders.h>
#include <ShlObj.h>
#include <cstdio>

#ifdef CreateDirectory
#undef CreateDirectory
#endif

#ifdef CopyFile
#undef CopyFile
#endif

namespace Fyrion
{
    DirIterator& DirIterator::operator++()
    {
        if (m_handler)
        {
            WIN32_FIND_DATA fd{};
            auto res = FindNextFile(m_handler, &fd);
            if (res != 0)
            {
                do
                {
                    bool isDirEntry = !(strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0);
                    if (isDirEntry)
                    {
                        m_path = m_directory + fd.cFileName;
                        return *this;
                    }
                } while (::FindNextFile(m_handler, &fd) != 0);
            }
            FindClose(m_handler);
            m_handler = nullptr;
        }
        m_path.Clear();
        return *this;
    }

	DirIterator::DirIterator(const StringView& directory) : m_directory(directory), m_handler(nullptr)
	{
		WIN32_FIND_DATA fd{};
		char cwd[MAX_PATH];
		sprintf(cwd, "%s\\*.*", directory.CStr());
		m_handler = FindFirstFile(cwd, &fd);

		if (m_handler != INVALID_HANDLE_VALUE)
		{
			do
			{
				bool isDirEntry = !(strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0);
				if (isDirEntry)
				{
					m_path = Path::Join(directory, fd.cFileName);
					break;
				}
			} while (::FindNextFile(m_handler, &fd) != 0);
		}
	}

	DirIterator::~DirIterator()
	{
		if (m_handler)
		{
			FindClose(m_handler);
		}
	}
}


namespace Fyrion::FileSystem
{

	String CurrentDir()
	{
		TCHAR path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, path);
		return {path, strlen(path)};
	}

    String DocumentsDir()
	{
		CHAR myDocuments[MAX_PATH];
		HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, myDocuments);
		return {myDocuments};
	}

	FileStatus GetFileStatus(const StringView& path)
	{
		WIN32_FILE_ATTRIBUTE_DATA fileAttrData = {0};
		bool exists = GetFileAttributesEx(path.CStr(), GetFileExInfoStandard, &fileAttrData);

        LARGE_INTEGER size{};
        size.HighPart = fileAttrData.nFileSizeHigh;
        size.LowPart = fileAttrData.nFileSizeLow;

        return FileStatus{
            .exists = exists,
            .isDirectory = (fileAttrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0,
            .lastModifiedTime = (u64) (static_cast<i64>(fileAttrData.ftLastWriteTime.dwHighDateTime) << 32 | fileAttrData.ftLastWriteTime.dwLowDateTime),
            .fileSize = (u64) size.QuadPart};
	}

    String AppFolder()
    {
        PWSTR pathTemp;
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathTemp);
        char buffer[MAX_PATH];
        usize size{};
        wcstombs_s(&size, buffer, pathTemp, MAX_PATH);
        CoTaskMemFree(pathTemp);
        return {buffer, size - 1};
    }

}

#endif