#pragma once

#include "Fyrion/Common.hpp"
#include "StringView.hpp"
namespace Fyrion
{
    struct TypeInfo
    {
        TypeID       typeId{};
        usize        size{};
        usize        alignment{};
    };

    template<typename Type>
    constexpr auto StrippedTypeName()
    {
        StringView prettyFunction = FY_PRETTY_FUNCTION;
        usize first = prettyFunction.FindFirstNotOf(' ', prettyFunction.FindFirstOf(FY_PRETTY_FUNCTION_PREFIX) + 1);
        StringView value = prettyFunction.Substr(first, prettyFunction.FindLastOf(FY_PRETTY_FUNCTION_SUFFIX) - first);
        return value;
    }

    template<typename Type>
    struct TypeIDGen
    {
        static constexpr auto GetTypeName()
        {
            StringView typeName = StrippedTypeName<Type>();

            usize space = typeName.FindFirstOf(' ');
            if (space != StringView::s_npos)
            {
                return typeName.Substr(space + 1);
            }
            return typeName;
        }

        constexpr static TypeID GetTypeID()
        {
            constexpr TypeID typeId = Hash<StringView>::Value(StrippedTypeName<Type>());
            return typeId;
        }
    };

    template<typename Type>
    constexpr static TypeID GetTypeID()
    {
        return TypeIDGen<Traits::RemoveAll<Type>>::GetTypeID();
    }

    template<typename Type>
    constexpr static StringView GetTypeName()
    {
        return TypeIDGen<Traits::RemoveAll<Type>>::GetTypeName();
    }

    template<typename Type>
    constexpr static usize GetTypeSize()
    {
        if constexpr (Traits::IsComplete<Traits::RemoveAll<Type>>)
        {
            return sizeof(Traits::RemoveAll<Type>);
        }
        return 0;
    }

    template<typename Type>
    constexpr static usize GetTypeAlign()
    {
        if constexpr (Traits::IsComplete<Traits::RemoveAll<Type>>)
        {
            return alignof(Traits::RemoveAll<Type>);
        }
        return 0;
    }

    template<typename Type>
    constexpr TypeInfo GetTypeInfo()
    {
        TypeInfo typeInfo = TypeInfo{
            .typeId = GetTypeID<Type>(),
            .size = GetTypeSize<Type>(),
            .alignment = GetTypeAlign<Type>()
        };
        return typeInfo;
    }
}