#include "StreamBuffer.hpp"

#include "Registry.hpp"
#include "Fyrion/IO/FileSystem.hpp"


namespace Fyrion
{
    Array<u8> StreamBuffer::Load() const
    {
        if (!inMemoryData.Empty())
        {
            return inMemoryData;
        }
        // Array<u8> bufferData = FileSystem::ReadFileAsByteArray(bufferFile);
        // if (!bufferData.Empty())
        // {
        //     Array<u8> ret;
        //     ret.Insert(ret.begin(), bufferData.begin() + offset, bufferData.begin() + offset + size);
        //     return ret;
        // }
        return {};
    }

    void StreamBuffer::Store(Span<u8> data)
    {
        inMemoryData = data;
        size = data.Size();
    }

    Span<u8> StreamBuffer::GetInMemoryData() const
    {
        return inMemoryData;
    }

    usize StreamBuffer::GetOffset() const
    {
        return offset;
    }

    usize StreamBuffer::GetSize() const
    {
        return size;
    }

    void StreamBuffer::RegisterType(NativeTypeHandler<StreamBuffer>& type)
    {
        type.Field<&StreamBuffer::offset>("offset");
        type.Field<&StreamBuffer::size>("size");
    }

    void StreamBuffer::FlushInMemoryData(ConstPtr data)
    {
        inMemoryData.Clear();
    }
}
