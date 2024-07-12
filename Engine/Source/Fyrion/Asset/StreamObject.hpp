#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/UUID.hpp"
#include "Fyrion/IO/FileTypes.hpp"

namespace Fyrion
{
    class FY_API StreamObject
    {
    public:
        void  Init(UUID uniqueId);
        void  Save(ConstPtr data, usize size);
        usize Size() const;
        void  Load(VoidPtr data) const;

        UUID GetId() const
        {
            return uniqueId;
        }
    private:
        UUID uniqueId;
    };

    template<>
    struct ArchiveType<StreamObject>
    {
        static void WriteField(ArchiveWriter& writer, ArchiveObject object, const StringView& name, const StreamObject& value)
        {
            char buffer[StringConverter<UUID>::bufferCount] = {};
            StringConverter<UUID>::ToString(buffer, 0, value.GetId());
            writer.WriteString(object, name, StringView{buffer, StringConverter<UUID>::bufferCount});
        }

        static void ReadField(ArchiveReader& reader, ArchiveObject object, const StringView& name, StreamObject& value)
        {
            value.Init(UUID::FromString(reader.ReadString(object, name)));
        }
    };
}
