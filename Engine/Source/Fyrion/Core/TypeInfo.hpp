#pragma once

#include "Fyrion/Common.hpp"
#include "StringView.hpp"
#include "TypeApiInfo.hpp"
#include "String.hpp"
#include "Serialization.hpp"

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
    typedef void (* FnExtractApi)(VoidPtr pointer);
    typedef usize (*FnStringSize)(ConstPtr pointer);
    typedef usize (*FnToString)(ConstPtr pointer, char* buffer);
    typedef void (* FnFromString)(VoidPtr pointer, const StringView& stringView);

    struct TypeInfo
    {
        TypeID         typeId;
        usize          size;
        usize          alignment;
        bool           isTriviallyCopyable;
        bool           isEnum;
        TypeID         apiId;
        FnExtractApi   extractApi;
        FnStringSize   stringSize;
        FnToString     toString;
        FnFromString   fromString;
        FnArchiveWrite archiveWrite;
        FnArchiveRead  archiveRead;
        FnArchiveAdd   archiveAdd;
        FnArchiveGet   archiveGet;
    };

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

    template <typename Type>
    constexpr static usize GetTypeSize()
    {
        if constexpr (Traits::IsComplete<Traits::RemoveAll<Type>>)
        {
            return sizeof(Traits::RemoveAll<Type>);
        }
        return 0;
    }

    template <typename Type>
    constexpr static usize GetTypeAlign()
    {
        if constexpr (Traits::IsComplete<Traits::RemoveAll<Type>>)
        {
            return alignof(Traits::RemoveAll<Type>);
        }
        return 0;
    }


    constexpr StringView GetSimpleName(const StringView name)
    {
        StringView ret{};
        Split(name, StringView{"::"}, [&](StringView str)
        {
            ret = str;
        });
        return ret;
    }

    template <typename Type>
    constexpr TypeInfo GetTypeInfo()
    {
        TypeInfo typeInfo = TypeInfo{
            .typeId = GetTypeID<Type>(),
            .size = GetTypeSize<Type>(),
            .alignment = GetTypeAlign<Type>(),
            .isTriviallyCopyable = Traits::IsTriviallyCopyable<Type>,
            .isEnum = Traits::IsEnum<Type>,
            .apiId = TypeApiInfo<Traits::RemoveAll<Type>>::GetApiId(),
            .extractApi = TypeApiInfo<Traits::RemoveAll<Type>>::ExtractApi,
        };

        if constexpr (StringConverter<Type>::hasConverter)
        {
            typeInfo.stringSize = [](ConstPtr pointer)
            {
                return StringConverter<Type>::Size(*static_cast<const Traits::RemoveAll<Type>*>(pointer));
            };

            typeInfo.toString = [](ConstPtr pointer, char* buffer)
            {
                return StringConverter<Type>::ToString(buffer, 0, *static_cast<const Traits::RemoveAll<Type>*>(pointer));
            };

            typeInfo.fromString = [](VoidPtr pointer, const StringView& stringView)
            {
                StringConverter<Type>::FromString(stringView.Data(), stringView.Size(), *static_cast<Traits::RemoveAll<Type>*>(pointer));
            };
        }

        if constexpr (ArchiveType<Type>::hasArchiveImpl)
        {
            typeInfo.archiveWrite = [](ArchiveWriter& writer, ArchiveObject object, StringView name, ConstPtr value)
            {
                ArchiveType<Type>::Write(writer, object, name, static_cast<const Type *>(value));
            };

            typeInfo.archiveRead = [](ArchiveReader& reader, ArchiveObject object, StringView name, VoidPtr value)
            {
                ArchiveType<Type>::Read(reader, object, name, static_cast<Type*>(value));
            };

            typeInfo.archiveAdd = [](ArchiveWriter& writer, ArchiveObject array, ConstPtr value)
            {
                ArchiveType<Type>::Add(writer, array, static_cast<const Type*>(value));
            };

            typeInfo.archiveGet = [](ArchiveReader& reader, ArchiveObject array, VoidPtr value)
            {
                ArchiveType<Type>::Get(reader, array, static_cast<Type*>(value));
            };
        }

        return typeInfo;
    }
}
