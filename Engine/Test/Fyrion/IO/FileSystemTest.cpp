#include <doctest.h>
#include "Fyrion/IO/FileSystem.hpp"

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
}