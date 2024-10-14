#pragma once

#include "Span.hpp"
#include "Array.hpp"
#include "Fyrion/Common.hpp"

#include "Serialization.hpp"

namespace Fyrion
{
    class FY_API StreamBuffer
    {
    public:
        Array<u8> Load() const;
        void      Store(Span<u8> data);

        void     FlushInMemoryData(ConstPtr data);
        Span<u8> GetInMemoryData() const;

        usize     offset = 0;
        usize     size = 0;
        Array<u8> inMemoryData;

        //TODO: temporary
        String bufferFile;

        static void RegisterType(NativeTypeHandler<StreamBuffer>& type);
    };


    // template <>
    // struct ArchiveType<StreamBuffer>
    // {
    //     constexpr static bool hasArchiveImpl = true;
    //
    //     static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, StreamBuffer* value)
    //     {
    //         Span<u8> data = value->GetInMemoryData();
    //         if (!data.Empty())
    //         {
    //             value->FlushInMemoryData(writer.WriteStream(object, name, data));
    //         }
    //     }
    //
    //     static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, StreamBuffer* value)
    //     {
    //
    //     }
    //
    //     static void Add(ArchiveWriter& writer, ArchiveObject array, const StreamBuffer* value)
    //     {
    //         FY_ASSERT(false, "not implemented yet");
    //     }
    //
    //     static void Get(ArchiveReader& reader, ArchiveObject item, StreamBuffer* value)
    //     {
    //         FY_ASSERT(false, "not implemented yet");
    //     }
    // };
}
