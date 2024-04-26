file(GLOB_RECURSE SPIRV_CROSS_SOURCES Source/*.hpp Source/*.cpp Source/*.h Source/*.c)
add_library(SPIRV-Cross STATIC ${SPIRV_CROSS_SOURCES})
target_include_directories(SPIRV-Cross PUBLIC Source)