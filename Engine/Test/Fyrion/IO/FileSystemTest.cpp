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
        CHECK(FileSystem::Remove("test"));
    }

    TEST_CASE("IO:FileSystemFile")
    {
        String path = Path::Join(FY_TEST_FILES, "TestWriteFile.txt");
        String testText = "texttexttextzzzz";

        {
            FileHandler fileHandler = FileSystem::OpenFile(path, AccessMode::WriteOnly);
            REQUIRE(fileHandler);
            CHECK(FileSystem::GetFileSize(fileHandler) == 0);
            FileSystem::WriteFile(fileHandler, testText.CStr(), testText.Size());
            FileSystem::CloseFile(fileHandler);
        }

        {
            FileHandler fileHandler = FileSystem::OpenFile(path, AccessMode::ReadOnly);
            REQUIRE(fileHandler);
            usize size = FileSystem::GetFileSize(fileHandler);
            CHECK(size == testText.Size());
            String newString{size};
            FileSystem::ReadFile(fileHandler, newString.begin(), size);
            CHECK(testText == newString);
            FileSystem::CloseFile(fileHandler);
        }

        CHECK(FileSystem::Remove(path));
    }

    TEST_CASE("IO:FileSystemMapFile")
    {
#ifdef FY_WIN
        constexpr usize fileSize = 128;

        String path = Path::Join(FY_TEST_FILES, "TestMapFile.bin");
        {
            FileHandler fileHandler = FileSystem::OpenFile(path, AccessMode::ReadAndWrite);
            CHECK(fileHandler);
            FileHandler mapFile = FileSystem::CreateFileMapping(fileHandler, AccessMode::ReadAndWrite, fileSize);
            CHECK(mapFile);

            CharPtr memory = (CharPtr) FileSystem::MapViewOfFile(mapFile);
            CHECK(mapFile);

            for (int i = 0; i < fileSize; ++i)
            {
                memory[i] = i * 2;
            }

            FileSystem::UnmapViewOfFile(memory);
            FileSystem::CloseFileMapping(mapFile);
            FileSystem::CloseFile(fileHandler);
        }

        {
            FileHandler fileHandler = FileSystem::OpenFile(path, AccessMode::ReadOnly);
            CHECK(fileHandler);
            FileHandler mapFile = FileSystem::CreateFileMapping(fileHandler, AccessMode::ReadOnly, 0);
            CHECK(mapFile);
            if (mapFile)
            {
                CharPtr memory = (CharPtr) FileSystem::MapViewOfFile(mapFile);
                CHECK(mapFile);
                if (memory)
                {
                    for (int i = 0; i < fileSize; ++i)
                    {
                        CHECK(memory[i] == i * 2);
                    }

                    FileSystem::UnmapViewOfFile(memory);
                }
                FileSystem::CloseFileMapping(mapFile);
            }
            FileSystem::CloseFile(fileHandler);
        }

        CHECK(FileSystem::Remove(path));
#endif
    }
}