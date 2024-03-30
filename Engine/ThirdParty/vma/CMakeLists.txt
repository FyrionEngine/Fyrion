add_library(vma INTERFACE)
target_include_directories(vma INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
