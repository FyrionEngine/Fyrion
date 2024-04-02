#include "ResourceSerialization.hpp"

namespace Fyrion
{
    void ResourceSerialization::ParseObject(const StringView& buffer, VoidPtr instance, TypeHandler* typeHandler)
    {

    }

    void ResourceSerialization::ParseResource(const StringView& buffer, RID rid)
    {

    }

    RID ResourceSerialization::ParseResourceInfo(const StringView& buffer)
    {
        return RID();
    }

    String ResourceSerialization::WriteObject(VoidPtr instance, TypeHandler* handler)
    {
        return Fyrion::String();
    }

    String ResourceSerialization::WriteResource(RID rid)
    {
        return Fyrion::String();
    }

    String ResourceSerialization::WriteResourceInfo(RID rid)
    {
        return Fyrion::String();
    }

}