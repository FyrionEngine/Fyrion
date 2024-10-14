#pragma once

#include "Span.hpp"
#include "Array.hpp"
#include "Fyrion/Common.hpp"

#include "Serialization.hpp"

namespace Fyrion
{
    struct FY_API StreamBufferReader
    {

    };


    class FY_API StreamBuffer
    {
    public:
        Array<u8> Load() const;
        void      Store(Span<u8> data);

        void     FlushInMemoryData(ConstPtr data);
        Span<u8> GetInMemoryData() const;


        usize GetOffset() const;
        usize GetSize() const;

    private:
        usize     offset = 0;
        usize     size = 0;
        Array<u8> inMemoryData;

        static void RegisterType(NativeTypeHandler<StreamBuffer>& type);
    };


    template <>
    struct ArchiveType<StreamBuffer>
    {
        constexpr static bool hasArchiveImpl = true;

        static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, StreamBuffer* value)
        {
            Span<u8> data = value->GetInMemoryData();
            if (!data.Empty())
            {
                writer.WriteStream(object, name, data);

                ArchiveObject stream = writer.CreateObject();
                writer.WriteUInt(stream, "offset", value->GetOffset());
                writer.WriteUInt(stream, "size", value->GetSize());
                writer.WriteValue(object, name, stream);
            }
        }

        static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, StreamBuffer* value)
        {

        }

        static void Add(ArchiveWriter& writer, ArchiveObject array, const StreamBuffer* value)
        {
            FY_ASSERT(false, "not implemented yet");
        }

        static void Get(ArchiveReader& reader, ArchiveObject item, StreamBuffer* value)
        {
            FY_ASSERT(false, "not implemented yet");
        }
    };
}
