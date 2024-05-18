#pragma once

#include <doctest.h>
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"

template <>
struct doctest::StringMaker<Fyrion::String>
{
    static String convert(const Fyrion::String& str)
    {
        return {str.CStr(), static_cast<String::size_type>(str.Size())};
    }
};

template <>
struct doctest::StringMaker<Fyrion::StringView>
{
    static String convert(const Fyrion::StringView& str)
    {
        return {str.CStr(), static_cast<String::size_type>(str.Size())};
    }
};
