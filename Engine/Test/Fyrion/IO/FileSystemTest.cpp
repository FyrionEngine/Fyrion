#include <doctest.h>
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

using namespace Fyrion;

namespace
{
    TEST_CASE("IO::FileSystem")
    {
        CHECK(!FileSystem::CurrentDir().Empty());
        CHECK(!FileSystem::AppFolder().Empty());
        CHECK(!FileSystem::DocumentsDir().Empty());
//        CHECK(!FileSystem::TempFolder().Empty());

        CHECK(FileSystem::CreateDirectory("test"));
        FileStatus status = FileSystem::GetFileStatus("test");
        CHECK(status.exists);
        CHECK(status.isDirectory);
        CHECK(status.lastModifiedTime > 0);
        CHECK(status.fileSize == 0);
        CHECK(FileSystem::Remove("test"));
    }

    TEST_CASE("IO:FileSystemFile")
    {
        String path = Path::Join(FY_TEST_FILES, "TestWriteFile");
        String testText = "texttexttext";

        {
            FileHandler fileHandler = FileSystem::OpenFile(path, AccessMode_WriteOnly);
            CHECK(fileHandler);
            CHECK(FileSystem::GetFileSize(fileHandler) == 0);
            FileSystem::WriteFile(fileHandler, testText.CStr(), testText.Size());
            FileSystem::CloseFile(fileHandler);
        }

        {
            FileHandler fileHandler = FileSystem::OpenFile(path, AccessMode_ReadOnly);
            CHECK(fileHandler);
            usize size = FileSystem::GetFileSize(fileHandler);
            CHECK(size == testText.Size());
            String newString{size};
            FileSystem::ReadFile(fileHandler, newString.begin(), size);
            CHECK(testText == newString);
            FileSystem::CloseFile(fileHandler);
        }

        CHECK(FileSystem::Remove(path));
    }
}