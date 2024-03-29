add_subdirectory(ThirdParty)


file(GLOB_RECURSE FYRION_ENGINE_SOURCES Source/*.hpp Source/*.cpp Source/*.h Source/*.c)
add_library(FyrionEngine SHARED ${FYRION_ENGINE_SOURCES})

target_include_directories(FyrionEngine PUBLIC Source)

target_link_libraries(FyrionEngine PUBLIC mimalloc)


target_compile_definitions(FyrionEngine PUBLIC FMT_LIB_EXPORT=1)


##tests
file(GLOB_RECURSE FYRION_ENGINE_TESTS_SOURCES Test/*.hpp Test/*.cpp Test/*.h Test/*.c)
add_executable(FyrionEngineTests ${FYRION_ENGINE_TESTS_SOURCES})

target_link_libraries(FyrionEngineTests PUBLIC FyrionEngine)

target_include_directories(FyrionEngineTests PUBLIC Thirdparty/doctest)
target_include_directories(FyrionEngineTests PUBLIC Tests)
target_compile_definitions(FyrionEngineTests PRIVATE FY_TEST_FILES="${CMAKE_CURRENT_SOURCE_DIR}/Test/Files")

add_test(NAME FyrionEngineTests COMMAND FyrionEngineTests)