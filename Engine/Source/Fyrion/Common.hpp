#pragma once

namespace Fyrion
{
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned long long  u64;
    typedef unsigned long int   ul32;

    typedef signed char         i8;
    typedef signed short        i16;
    typedef signed int          i32;
    typedef signed long long    i64;

    typedef float               f32;
    typedef double              f64;

    typedef void*               VoidPtr;
    typedef const void*         ConstPtr;
    typedef char*               CharPtr;
    typedef u64                 TypeID;
    typedef decltype(sizeof(0)) usize;

    typedef f32                 Float;

    struct PlaceHolder
    {
    };

    static constexpr usize nPos = -1;

    #define FY_HANDLER(StructName) struct StructName {                                  \
        VoidPtr handler;                                                                \
     operator bool() const {return handler != nullptr; }                                \
     bool operator==(const StructName& b) const { return this->handler == b.handler; }  \
     bool operator!=(const StructName& b) const { return this->handler != b.handler; }  \
    }
}

inline void* operator new(Fyrion::usize, Fyrion::PlaceHolder, Fyrion::VoidPtr ptr)
{
    return ptr;
}

inline void operator delete(void*, Fyrion::PlaceHolder, Fyrion::VoidPtr) noexcept
{
}
//defines

//--general defines
#define FY_STRING_BUFFER_SIZE 18
#define FY_FRAMES_IN_FLIGHT 2
#define FY_REPO_PAGE_SIZE 4096
#define FY_ASSET_EXTENSION ".fy_asset"
#define FY_DATA_EXTENSION ".fy_data"
#define FY_CHUNK_COMPONENT_SIZE (16*1024)

//---platform defines
#if _WIN64
    #define FY_API __declspec(dllexport)
    #define FY_PATH_SEPARATOR '\\'
    #define FY_SHARED_EXT ".dll"
    #define FY_WIN
    #define FY_DESKTOP
#elif __linux__
    #define FY_API __attribute__ ((visibility ("default")))
    #define FY_PATH_SEPARATOR '/'
    #define FY_SHARED_EXT ".so"
    #define FY_LINUX
    #define FY_DESKTOP  //TODO android?
#elif __APPLE__
    #define FY_API
    #define FY_PATH_SEPARATOR '/'
    #define FY_SHARED_EXT ".dylib"

    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define FY_IOS
    #elif TARGET_OS_MAC
        #define FY_DESKTOP
        #define FY_MACOS
    #endif
#endif

#ifdef __unix__
    #define FY_UNIX
#endif

#ifndef FY_PRETTY_FUNCTION
#if defined _MSC_VER
#   define FY_PRETTY_FUNCTION __FUNCSIG__
#   define FY_PRETTY_FUNCTION_PREFIX '<'
#   define FY_PRETTY_FUNCTION_SUFFIX '>'
#elif defined __clang__ || defined __GNUC__
#   define FY_PRETTY_FUNCTION __PRETTY_FUNCTION__
#   define FY_PRETTY_FUNCTION_PREFIX '='
#   define FY_PRETTY_FUNCTION_SUFFIX ']'
#endif
#endif

#if defined _MSC_VER
    #define FY_FINLINE __forceinline
#elif defined __CLANG__
#       define FY_FINLINE [[clang::always_inline]]
#elif  defined__GNUC__
#define FY_FINLINE inline __attribute__((always_inline))
#else
static_assert(false, "Compiler not supported");
#endif




#if defined _MSC_VER
//unsigned int MAX
#   define U8_MAX           0xffui8
#   define U16_MAX          0xffffui16
#   define U32_MAX          0xffffffffui32
#   define U64_MAX          0xffffffffffffffffui64

//signed int MIN
#   define I8_MIN           (-127i8 - 1)
#   define I16_MIN          (-32767i16 - 1)
#   define I32_MIN          (-2147483647i32 - 1)
#   define I64_MIN          (-9223372036854775807i64 - 1)

//signed int MAX
#   define I8_MAX           127i8
#   define I16_MAX          32767i16
#   define I32_MAX          2147483647i32
#   define I64_MAX          9223372036854775807i64

#   define F32_MAX          3.402823466e+38F
#   define F64_MAX          1.7976931348623158e+308

#   define F32_MIN          1.175494351e-38F
#   define F64_MIN          2.2250738585072014e-308

#   define F32_LOW          (-(F32_MAX))
#   define F64_LOW          (-(F64_MAX))

#elif defined __GNUC__
# define I8_MIN		    (-128)
# define I16_MIN		(-32767-1)
# define I32_MIN		(-2147483647-1)
# define I64_MIN	    INT64_MIN

# define I8_MAX		    (127)
# define I16_MAX		(32767)
# define I32_MAX		(2147483647)
# define I64_MAX		INT64_MAX

/* Maximum of unsigned integral types.  */
# define U8_MAX		    (255)
# define U16_MAX		(65535)
# define U32_MAX		(4294967295U)
# define U64_MAX		18446744073709551615UL

# define F32_MAX        __FLT_MAX__
# define F64_MAX        __DBL_MAX__

# define F32_MIN        __FLT_MIN__
# define F64_MIN        __DBL_MIN__

# define F32_LOW         (-(F32_MAX))
# define F64_LOW         (-(F64_MAX))
#endif

#define ENUM_FLAGS(ENUMNAME, ENUMTYPE) \
inline ENUMNAME& operator |= (ENUMNAME& a, ENUMNAME b)  noexcept { return (ENUMNAME&)(((ENUMTYPE&)a) |= ((ENUMTYPE)b)); } \
inline ENUMNAME& operator &= (ENUMNAME& a, ENUMNAME b)  noexcept { return (ENUMNAME&)(((ENUMTYPE&)a) &= ((ENUMTYPE)b)); } \
inline ENUMNAME& operator ^= (ENUMNAME& a, ENUMNAME b)  noexcept { return (ENUMNAME&)(((ENUMTYPE&)a) ^= ((ENUMTYPE)b)); } \
inline ENUMNAME& operator <<= (ENUMNAME& a, ENUMTYPE b)  noexcept { return (ENUMNAME&)(((ENUMTYPE&)a) <<= ((ENUMTYPE)b)); } \
inline ENUMNAME& operator >>= (ENUMNAME& a, ENUMTYPE b)  noexcept { return (ENUMNAME&)(((ENUMTYPE&)a) >>= ((ENUMTYPE)b)); } \
inline ENUMNAME operator | (ENUMNAME a, ENUMNAME b)    noexcept { return ENUMNAME(((ENUMTYPE)a) | ((ENUMTYPE)b));        } \
inline ENUMNAME operator & (ENUMNAME a, ENUMNAME b)    noexcept { return ENUMNAME(((ENUMTYPE)a) & ((ENUMTYPE)b));        } \
inline bool   operator && (ENUMNAME a, ENUMNAME b)    noexcept { return ((ENUMTYPE)a & ((ENUMTYPE)b));            } \
inline ENUMNAME operator ~ (ENUMNAME a)                noexcept { return ENUMNAME(~((ENUMTYPE)a));                        } \
inline ENUMNAME operator ^ (ENUMNAME a, ENUMNAME b)    noexcept { return ENUMNAME(((ENUMTYPE)a) ^ (ENUMTYPE)b);        }  \

#ifdef NDEBUG
#  define FY_ASSERT(condition, message) ((void)0)
#else
#  include <cassert>
#  define FY_ASSERT(condition, message) assert(condition && message)
#  define FY_DEBUG
#endif