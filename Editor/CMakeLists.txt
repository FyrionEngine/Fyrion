file(GLOB_RECURSE FYRION_EDITOR_SOURCES Source/*.hpp Source/*.cpp Source/*.h Source/*.c)
add_library(FyrionEditor SHARED ${FYRION_EDITOR_SOURCES})

target_include_directories(FyrionEditor PUBLIC Source)

target_link_libraries(FyrionEditor PUBLIC FyrionEngine)