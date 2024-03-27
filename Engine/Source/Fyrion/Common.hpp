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

#define FY_API __declspec(dllexport)
#define FY_FINLINE
#define FY_STRING_BUFFER_SIZE 18