#pragma once

#include "Fyrion/Common.hpp"
#include "StringView.hpp"
#include "TypeApiInfo.hpp"
#include "String.hpp"
#include "TypeID.hpp"


namespace Fyrion
{
    typedef void (* FnExtractApi)(VoidPtr pointer);

    struct ArchiveWriter;
    struct ArchiveReader;

    typedef ArchiveValue (*FnArchiveToValue)(ArchiveWriter& writer, ConstPtr value);
    typedef void         (*FnArchiveFromValue)(ArchiveReader& reader, ArchiveValue value, VoidPtr retValue);

    struct TypeInfo
    {
        TypeID             typeId;
        usize              size;
        usize              alignment;
        bool               isTriviallyCopyable;
        bool               isEnum;
        TypeID             apiId;
        FnExtractApi       extractApi;
        FnArchiveToValue   archiveToValue;
        FnArchiveFromValue archiveFromValue;
    };

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

        if constexpr (ArchiveType<Type>::hasArchiveImpl)
        {
            typeInfo.archiveToValue = [](ArchiveWriter& writer, ConstPtr value)
            {
                return ArchiveType<Type>::ToValue(writer, *static_cast<const Type*>(value));
            };

            typeInfo.archiveFromValue = [](ArchiveReader& reader, ArchiveValue value, VoidPtr retValue)
            {
                ArchiveType<Type>::FromValue(reader, value, *static_cast<Type*>(retValue));
            };
        }

        return typeInfo;
    }
}
