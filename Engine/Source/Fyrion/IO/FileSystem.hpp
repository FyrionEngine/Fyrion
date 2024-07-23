#pragma once

#include "FileTypes.hpp"
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Array.hpp"

namespace Fyrion::FileSystem
{
    FY_API String CurrentDir();
    FY_API String DocumentsDir();
    FY_API String AppFolder();
    FY_API String AssetFolder();
    FY_API String TempFolder();

    FY_API FileStatus GetFileStatus(const StringView& path);
    FY_API bool       CreateDirectory(const StringView& path);
    FY_API bool       Remove(const StringView& path);
    FY_API bool       Rename(const StringView& oldName, const StringView& newName);
    FY_API bool       CopyFile(const StringView& from, const StringView& to);

    FY_API FileHandler OpenFile(const StringView& path, AccessMode accessMode);
    FY_API u64         GetFileSize(FileHandler fileHandler);
    FY_API u64         WriteFile(FileHandler fileHandler, ConstPtr data, usize size);
    FY_API u64         ReadFile(FileHandler fileHandler, VoidPtr data, usize size);
    FY_API void        CloseFile(FileHandler fileHandler);
    FY_API FileHandler CreateFileMapping(FileHandler fileHandler, AccessMode accessMode, usize size);
    FY_API VoidPtr     MapViewOfFile(FileHandler fileHandler);
    FY_API bool        UnmapViewOfFile(VoidPtr map);
    FY_API void        CloseFileMapping(FileHandler fileHandler);

    FY_API String    ReadFileAsString(const StringView& path);
    FY_API Array<u8> ReadFileAsByteArray(const StringView& path);
    FY_API void      SaveFileAsString(const StringView& path, const StringView& string);
}
