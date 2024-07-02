add_library(yyjson src/yyjson.h src/yyjson.c)
target_include_directories(yyjson PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)


if (XCODE)
	set(YYJSON_FLAGS "-Wall -Wextra -Werror -pedantic -pedantic-errors")
	if (YYJSON_ENABLE_FASTMATH)
		set(YYJSON_FLAGS "${YYJSON_FLAGS} -ffast-math")
	endif ()

	set_default_xcode_property(yyjson)
	set_xcode_deployment_version(yyjson "10.13" "12.0" "12.0" "4.0")

	set_xcode_property(yyjson GCC_C_LANGUAGE_STANDARD "c89")
	set_xcode_property(yyjson CLANG_CXX_LANGUAGE_STANDARD "c++98")

	set_xcode_property(yyjson OTHER_CFLAGS[variant=Debug] ${YYJSON_FLAGS})
	set_xcode_property(yyjson OTHER_CFLAGS[variant=MinSizeRel] ${YYJSON_FLAGS})
	set_xcode_property(yyjson OTHER_CFLAGS[variant=RelWithDebInfo] ${YYJSON_FLAGS})
	set_xcode_property(yyjson OTHER_CFLAGS[variant=Release] ${YYJSON_FLAGS})

elseif (MSVC)
	set(YYJSON_FLAGS "/utf-8")
	if (YYJSON_ENABLE_FASTMATH)
		set(YYJSON_FLAGS "${YYJSON_FLAGS} /fp:fast")
	endif ()

	target_compile_options(yyjson PRIVATE $<$<C_COMPILER_ID:MSVC>:${YYJSON_FLAGS}>)
	target_compile_options(yyjson PRIVATE $<$<CXX_COMPILER_ID:MSVC>:${YYJSON_FLAGS}>)

elseif (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|Intel")

	set(YYJSON_FLAGS "")
	if (YYJSON_ENABLE_FASTMATH)
		set(YYJSON_FLAGS "-ffast-math")
	endif ()

	target_compile_options(yyjson PRIVATE $<$<COMPILE_LANGUAGE:C>:${YYJSON_FLAGS}>)
	target_compile_options(yyjson PRIVATE $<$<COMPILE_LANGUAGE:CXX>:${YYJSON_FLAGS}>)

endif ()