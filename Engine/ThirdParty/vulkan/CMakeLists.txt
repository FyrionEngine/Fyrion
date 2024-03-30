add_library(vulkan-sdk INTERFACE)
target_include_directories(vulkan-sdk INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)