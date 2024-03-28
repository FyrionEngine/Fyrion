add_library(mimalloc src/static.c)
target_include_directories(mimalloc PUBLIC include)


target_compile_definitions(mimalloc PRIVATE MIMALLOC_SHOW_STATS=1)

