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
    typedef u64                 TypeID;
    typedef decltype(sizeof(0)) usize;

    struct PlaceHolder
    {
    };

    static constexpr usize nPos = -1;
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


//---platform defines
#if _WIN64
    #define FY_API __declspec(dllexport)
    #define FY_PATH_SEPARATOR '\\'
    #define FY_FINLINE __forceinline
    #define FY_SHARED_EXT ".dll"
    #define FY_WIN
#elif __linux__
    #define FY_API __attribute__ ((visibility ("default")))
    #define FY_PATH_SEPARATOR '/'
    #define FY_SHARED_EXT ".so"
    #define FY_FINLINE inline
    #define FY_LINUX
#elif __APPLE__
    #define FY_API
    #define FY_PATH_SEPARATOR '/'
    #define FY_FINLINE static inline
    #define FY_SHARED_EXT ".dylib"

    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define FY_IOS
    #elif TARGET_OS_MAC
        #define FY_MACOS
    #endif
#endif

#ifdef __unix__
    #define FY_UNIX
#endif