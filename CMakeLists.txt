cmake_minimum_required(VERSION 3.25)
project(Fyrion)

set(CMAKE_CXX_STANDARD 20)

include(CMake/CMakeFunctions.cmake)

string(TIMESTAMP TODAY "%Y%m%d")
add_compile_definitions(FY_VERSION="0.0.1-${TODAY}-pre-alpha")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_CXX_COMPILER_ID MATCHES "GNU"))
	add_compile_options(-fms-extensions)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++20")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

if ( MSVC )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)
endif (MSVC)

if (UNIX AND NOT APPLE)
	set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS_INIT} -fPIC")
	set(CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS_INIT} -fPIC")
	SET(CMAKE_SKIP_BUILD_RPATH FALSE)
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	SET(CMAKE_INSTALL_RPATH "\${ORIGIN}")
elseif (APPLE)
	set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS_INIT} -fPIC")
	set(CMAKE_C_FLAGS                  "${CMAKE_C_FLAGS_INIT} -fPIC")
	SET(CMAKE_SKIP_BUILD_RPATH FALSE)
	SET(CMAKE_INSTALL_RPATH "${ORIGIN}/")
endif()

enable_testing()

add_subdirectory(Editor)
add_subdirectory(Engine)
add_subdirectory(Fyrion)
add_subdirectory(Player)


