add_library(volk src/volk.c)
target_include_directories(volk PUBLIC src)
target_link_libraries(volk PRIVATE vulkan-sdk)