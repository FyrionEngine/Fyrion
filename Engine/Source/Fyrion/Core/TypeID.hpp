#pragma once


#include "StringView.hpp"

template <typename Type>
constexpr auto Fyrion_StrippedTypeName()
{
    Fyrion::StringView prettyFunction = FY_PRETTY_FUNCTION;
    Fyrion::usize      first = prettyFunction.FindFirstNotOf(' ', prettyFunction.FindFirstOf(FY_PRETTY_FUNCTION_PREFIX) + 1);
    Fyrion::StringView value = prettyFunction.Substr(first, prettyFunction.FindLastOf(FY_PRETTY_FUNCTION_SUFFIX) - first);
    return value;
}

namespace Fyrion
{
    template <typename Type>
    struct TypeIDGen
    {
        static constexpr auto GetTypeName()
        {
            StringView typeName = Fyrion_StrippedTypeName<Type>();

            usize space = typeName.FindFirstOf(' ');
            if (space != StringView::s_npos)
            {
                return typeName.Substr(space + 1);
            }
            return typeName;
        }

        constexpr static TypeID GetTypeID()
        {
            constexpr TypeID typeId = Hash<StringView>::Value(Fyrion_StrippedTypeName<Type>());
            return typeId;
        }
    };

    template <typename Type>
    constexpr static TypeID GetTypeID()
    {
        return TypeIDGen<Traits::RemoveAll<Type>>::GetTypeID();
    }

    template <typename Type>
    constexpr static StringView GetTypeName()
    {
        return TypeIDGen<Traits::RemoveAll<Type>>::GetTypeName();
    }
}
