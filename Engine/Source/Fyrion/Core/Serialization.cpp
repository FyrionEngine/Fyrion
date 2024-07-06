#include "Serialization.hpp"

#include "Registry.hpp"

namespace Fyrion
{
    ArchiveObject Serialization::Serialize(const TypeHandler* typeHandler, ArchiveWriter& writer, ConstPtr instance)
    {
        const ArchiveObject object = writer.CreateObject();
        for (FieldHandler* field : typeHandler->GetFields())
        {
            field->Serialize(writer, instance, object);
        }
        return object;
    }

    void Serialization::Deserialize(const TypeHandler* typeHandler, ArchiveReader& reader, ArchiveObject object, VoidPtr instance)
    {
        for (FieldHandler* field : typeHandler->GetFields())
        {
            field->Deserialize(reader, instance, object);
        }
    }
}
