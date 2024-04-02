#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Core/Span.hpp"

namespace Fyrion
{
    //foward declarations

    struct ResourceStorage;
    struct ResourceObject;
    struct ResourceData;
    struct ResourceType;

    //types
    struct RID
    {
        union
        {
            struct
            {
                u32 offset;
                u32 page;
            };
            u64 id{};
        };

        inline explicit operator bool() const noexcept
        {
            return this->offset > 0 || this->page > 0;
        }

        inline bool operator==(const RID& rid) const
        {
            return this->id == rid.id;
        }

        inline bool operator!=(const RID& rid) const
        {
            return !((*this) == rid);
        }
    };

    template<>
    struct Hash<RID>
    {
        constexpr static bool hasHash = true;
        constexpr static usize Value(const RID& rid)
        {
            return Hash<u64>::Value(rid.id);
        }
    };

    enum class ResourceFieldType : u16
    {
        Value = 1,
        SubObject = 2,
        SubObjectSet = 3,
        Stream = 4
    };

    ENUM_FLAGS(ResourceFieldType, u16);

    struct ResourceFieldCreation
    {
        u32 index{U32_MAX};
        StringView name{};
        ResourceFieldType type{};
        TypeID valueId{};
    };

    struct ResourceTypeCreation
    {
        StringView name{};
        TypeID typeId{};
        Span<ResourceFieldCreation> fields{};
    };


    typedef RID (*FnImportAsset)(RID asset, const StringView& path);

    struct AssetRoot
    {
        constexpr static u32 Name = 0;
        constexpr static u32 Assets = 1;
        constexpr static u32 Directories = 2;
        constexpr static u32 Path = 3;
    };

    struct AssetDirectory
    {
        constexpr static u32 Name = 0;
        constexpr static u32 Parent = 1;
        constexpr static u32 Path = 2;
    };

    struct Asset
    {
        constexpr static u32 Name = 0;
        constexpr static u32 Directory = 1;
        constexpr static u32 Object = 2;
        constexpr static u32 Path = 3;
        constexpr static u32 Extension = 4;
    };


}