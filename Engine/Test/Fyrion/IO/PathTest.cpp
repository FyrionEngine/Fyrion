
#include <doctest.h>
#include "Fyrion/IO/Path.hpp"

using namespace Fyrion;

namespace
{
    TEST_CASE("IO::PathBasics")
    {
        String check{};
        check += "C:";
        check += FY_PATH_SEPARATOR;
        check += "Folder1";
        check += FY_PATH_SEPARATOR;
        check += "Folder2";
        check += FY_PATH_SEPARATOR;
        check += "Folder3";
        check += FY_PATH_SEPARATOR;
        check += "Folder4";

        String parent = Path::Join("C:/", "Folder1/", "/Folder2", "Folder3/Folder4");
        CHECK(!parent.Empty());
        CHECK(check == parent);

        String file = Path::Join(parent, "Leaf.exe");

        check += FY_PATH_SEPARATOR;
        check += "Leaf.exe";
        CHECK(file == check);

        CHECK(!file.Empty());
        CHECK(Path::Extension(file) == ".exe");
        CHECK(Path::Name(file) == "Leaf");
        CHECK(Path::Parent(file) == parent);
    }
}