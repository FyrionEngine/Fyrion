add_subdirectory(ThirdParty)

set(GLFW_BUILD_WAYLAND OFF)

file(GLOB_RECURSE FYRION_ENGINE_SOURCES Source/*.hpp Source/*.cpp Source/*.h Source/*.c)
add_library(FyrionEngine SHARED ${FYRION_ENGINE_SOURCES})

target_include_directories(FyrionEngine PUBLIC Source)
target_include_directories(FyrionEngine PUBLIC Source/Fyrion/ImGui/Lib)
target_include_directories(FyrionEngine PRIVATE ThirdParty/concurrentqueue)
target_include_directories(FyrionEngine PRIVATE ThirdParty/dxc/include)
target_include_directories(FyrionEngine PRIVATE ThirdParty/filewatch)

target_compile_definitions(FyrionEngine PUBLIC FMT_LIB_EXPORT=1)

target_link_libraries(FyrionEngine PRIVATE mimalloc glfw vma volk vulkan-sdk freetype nfd SPIRV-Reflect cpptrace::cpptrace yyjson stb cgltf mikktspace)

add_binary_file(FyrionEngine ThirdParty/dxc dxcompiler)

##tests
file(GLOB_RECURSE FYRION_ENGINE_TESTS_SOURCES Test/*.hpp Test/*.cpp Test/*.h Test/*.c)
add_executable(FyrionEngineTests ${FYRION_ENGINE_TESTS_SOURCES})

target_link_libraries(FyrionEngineTests PUBLIC FyrionEngine)

target_include_directories(FyrionEngineTests PUBLIC ThirdParty/doctest)
target_include_directories(FyrionEngineTests PUBLIC Test)
target_compile_definitions(FyrionEngineTests PRIVATE FY_TEST_FILES="${CMAKE_CURRENT_SOURCE_DIR}/Test/Files")

add_test(NAME FyrionEngineTests COMMAND FyrionEngineTests)